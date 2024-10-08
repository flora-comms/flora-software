//! Embassy access point
//!
//! - creates an open access-point with SSID `esp-wifi`
//! - you can connect to it using a static IP in range 192.168.2.2 .. 192.168.2.255, gateway 192.168.2.1
//! - open http://192.168.2.1:8080/ in your browser - the example will perform an HTTP get request to some "random" server
//!
//! On Android you might need to choose _Keep Accesspoint_ when it tells you the WiFi has no internet connection, Chrome might not want to load the URL - you can use a shell and try `curl` and `ping`
//!
//! Because of the huge task-arena size configured this won't work on ESP32-S2

//% FEATURES: embassy embassy-generic-timers esp-wifi esp-wifi/async esp-wifi/embassy-net esp-wifi/wifi-default esp-wifi/wifi esp-wifi/utils
//% CHIPS: esp32 esp32s2 esp32s3 esp32c2 esp32c3 esp32c6

#![no_std]
#![no_main]

use embassy_executor::Spawner;
use embassy_net::{
    tcp::TcpSocket,
    IpListenEndpoint,
    Ipv4Address,
    Ipv4Cidr,
    Stack,
    StackResources,
    StaticConfigV4,
};
use embassy_embedded_hal::adapter::;
use embassy_time::{Duration, Timer};
use esp_backtrace as _;
use esp_hal::{clock::ClockControl, dma::*, dma_buffers, gpio::Io, peripherals, prelude::_fugit_RateExtU32, rng::Rng, spi::{master::{Spi, SpiDma}, SpiMode}, system::SystemControl, timer::timg::TimerGroup};
use esp_println::{print, println};
use esp_wifi::{
    initialize, wifi::{
        AccessPointConfiguration,
        Configuration,
        WifiApDevice,
        WifiController,
        WifiDevice,
        WifiEvent,
        WifiState,
    }, wifi_interface::Socket, EspWifiInitFor
};
use picoserve::{response::{Directory, File}, routing::get_service};
use embedded_sdmmc::{self, SdCard};

macro_rules! mk_static {
    ($t:ty,$val:expr) => {{
        static STATIC_CELL: static_cell::StaticCell<$t> = static_cell::StaticCell::new();
        #[deny(unused_attributes)]
        let x = STATIC_CELL.uninit().write(($val));
        x
    }};
}


#[esp_hal_embassy::main]
async fn main(spawner: Spawner) -> ! {
    esp_println::logger::init_logger_from_env();
    let peripherals = peripherals::Peripherals::take();
    let system = SystemControl::new(peripherals.SYSTEM);
    let clocks = ClockControl::max(system.clock_control).freeze();
    let timg0 = TimerGroup::new(peripherals.TIMG0, &clocks);

    let init = initialize(
        EspWifiInitFor::Wifi,
        timg0.timer0,
        Rng::new(peripherals.RNG),
        peripherals.RADIO_CLK,
        &clocks
    )
    .unwrap();

    let wifi = peripherals.WIFI;
    let (wifi_interface, controller) =
        esp_wifi::wifi::new_with_mode(&init, wifi, WifiApDevice).unwrap();

    use esp_hal::timer::systimer::{SystemTimer, Target};
    let systimer = SystemTimer::new(peripherals.SYSTIMER).split::<Target>();
    esp_hal_embassy::init(&clocks, systimer.alarm0);
        
    

    let config = embassy_net::Config::ipv4_static(StaticConfigV4 {
        address: Ipv4Cidr::new(Ipv4Address::new(192, 168, 2, 1), 24),
        gateway: Some(Ipv4Address::from_bytes(&[192, 168, 2, 1])),
        dns_servers: Default::default(),
    });

    let seed = 1234; // very random, very secure seed

    // Init network stack
    let stack = &*mk_static!(
        Stack<WifiDevice<'_, WifiApDevice>>,
        Stack::new(
            wifi_interface,
            config,
            mk_static!(StackResources<3>, StackResources::<3>::new()),
            seed
        )
    );

    spawner.spawn(connection(controller)).ok();
    spawner.spawn(net_task(&stack)).ok();

    

    loop {
        if stack.is_link_up() {
            break;
        }
        Timer::after(Duration::from_millis(500)).await;
    }
    println!("Connect to the AP `esp-wifi` and point your browser to http://192.168.2.1:8080/");
    println!("Use a static IP in the range 192.168.2.2 .. 192.168.2.255, use gateway 192.168.2.1");

    // config dma spi
    let io = Io::new(peripherals.GPIO, peripherals.IO_MUX);
    let sclk = io.pins.gpio0;
    let miso = io.pins.gpio2;
    let mosi = io.pins.gpio4;
    let cs = io.pins.gpio5;

    let dma = Dma::new(peripherals.DMA);

    let dma_channel = dma.channel0;

    let (rx_buffer, rx_descriptors, tx_buffer, tx_descriptors) = dma_buffers!(32000);
    let dma_rx_buf = DmaRxBuf::new(rx_descriptors, rx_buffer).unwrap();
    let dma_tx_buf = DmaTxBuf::new(tx_descriptors, tx_buffer).unwrap();
    let mut sd_delay = Delay::new(&clocks);

    let mut sd_spi = Spi::new(peripherals.SPI2, 25u32.MHz(), SpiMode::Mode0, &clocks)
        .with_pins(Some(sclk), Some(mosi), Some(miso), Some(cs));

    let sdcard = embedded_sdmmc::sdcard::SdCard::new_with_options(spi, delayer, options);

    spawner.spawn(web_server(&stack));

    loop {}
        
}


#[embassy_executor::task]
async fn connection(mut controller: WifiController<'static>) {
    println!("start connection task");
    println!("Device capabilities: {:?}", controller.get_capabilities());
    loop {
        match esp_wifi::wifi::get_wifi_state() {
            WifiState::ApStarted => {
                // wait until we're no longer connected
                controller.wait_for_event(WifiEvent::ApStop).await;
                Timer::after(Duration::from_millis(5000)).await
            }
            _ => {}
        }
        if !matches!(controller.is_started(), Ok(true)) {
            let client_config = Configuration::AccessPoint(AccessPointConfiguration {
                ssid: "esp-wifi".try_into().unwrap(),
                ..Default::default()
            });
            controller.set_configuration(&client_config).unwrap();
            println!("Starting wifi");
            controller.start().await.unwrap();
            println!("Wifi started!");
        }
    }
}

#[embassy_executor::task]
async fn net_task(stack: &'static Stack<WifiDevice<'static, WifiApDevice>>) {
    stack.run().await
}

#[embassy_executor::task]
async fn web_server(stack: &'static Stack<WifiDevice<'_, WifiApDevice>>, sdcard: SdCard<>) {
    let mut rx_buffer = [0u8; 1536];
    let mut tx_buffer = [0u8; 1536];
    let mut socket: TcpSocket<'_> = unsafe { TcpSocket::new(stack, &mut rx_buffer, &mut tx_buffer) };
    socket.set_timeout(Some(embassy_time::Duration::from_secs(10)));
    println!("Wait for connection...");
    let r = socket
        .accept(IpListenEndpoint {
            addr: None,
            port: 80,
        })
        .await;
    println!("Connected...");

    // get files from sd card
    


    let app = picoserve::Router::new()
        .route("/", get_service(File::html()))
        .nest_service("/static", const {
            Directory {
                files: &[("index.css", File::css(include_str!("index.css")))],
                ..Directory::DEFAULT
            }
        },
        );
    
    let piconfig = picoserve::Config::new(picoserve::Timeouts {
        start_read_request: Some(Duration::from_secs(5)),
        read_request: Some(Duration::from_secs(1)),
        write: Some(Duration::from_secs(1)),
    }).keep_connection_alive();

    match picoserve::serve(&app, &piconfig, &mut [0; 2048], socket).await {
                    Ok(handled_requests_count) => {
                        println!(
                            "{handled_requests_count} requests handled."
                        )
                    }
                    Err(err) => println!("{err:?}"),
                }

    if let Err(e) = r {
        println!("connect error: {:?}", e);
    }
}