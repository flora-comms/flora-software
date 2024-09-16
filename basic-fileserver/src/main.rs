#![no_std]
#![no_main]

use embassy_executor::Spawner;
use embassy_net::{Ipv4Cidr, Stack as NetStack, StackResources, StaticConfigV4};
use embassy_time::{Duration, Timer};
use esp_backtrace as _;
use esp_hal::{
    clock::ClockControl, 
    // for use with second core...
    // cpu_control::Stack as CpuStack, 
    gpio::Io, 
    peripherals::Peripherals, 
    system::SystemControl, 
    timer::timg::TimerGroup
};
use esp_println::println;
use esp_wifi::wifi::{AccessPointConfiguration, Configuration, WifiApDevice, WifiController, WifiDevice, WifiEvent, WifiState};
// for use with second core
// static mut APP_CORE_STACK: CpuStack<8192> = CpuStack::new();

macro_rules! mk_static {
    ($t:ty,$val:expr) => {{
        static STATIC_CELL: static_cell::StaticCell<$t> = static_cell::StaticCell::new();
        #[deny(unused_attributes)]
        let x = STATIC_CELL.uninit().write(($val));
        x
    }};
}

// connection handler
#[embassy_executor::task]
async fn new_connection(mut controller: WifiController<'static>) {
    println!("Start connection task");
    println!("Device capabilities: {:?}", controller.get_capabilities());
    loop {
        match esp_wifi::wifi::get_wifi_state() {
            WifiState::ApStarted => {
                // wait until no connection
                controller.wait_for_event(WifiEvent::ApStop).await;
                Timer::after(Duration::from_millis(5000)).await;
            }
           
           _ => {}
        }
        if !matches!(controller.is_started(), Ok(true)) {
            let client_config = Configuration::AccessPoint(AccessPointConfiguration {
                ssid: "esp-wifi".try_into().unwrap(),
                ..Default::default()
            });
            controller.set_configuration(&client_config).unwrap();
            println!("Starting Wifi");
            controller.start().await.unwrap();
            println!("Wifi Started!");
        }
    }
}

#[embassy_executor::task]
async fn net_task(stack: &'static NetStack<WifiDevice<'static, WifiApDevice>>) {
    stack.run().await;
}

#[esp_hal_embassy::main]
async fn main(spawner: Spawner) -> ! {
    let peripherals = Peripherals::take();
    let system = SystemControl::new(peripherals.SYSTEM);
    // let io = Io::new(peripherals.GPIO, peripherals.IO_MUX);
    let clocks = ClockControl::max(system.clock_control).freeze();

    esp_println::logger::init_logger_from_env();

    // init timer group 0 for wifi and timer group 1 for the executor
    let timg0 = TimerGroup::new(peripherals.TIMG0, &clocks);  
    let timg1 = TimerGroup::new(peripherals.TIMG1, &clocks);

    // create a hardware timer for use by embassy
    let em_timer0 = timg1.timer0;
    // wifi initialization
    let wifi_init = esp_wifi::initialize(
        esp_wifi::EspWifiInitFor::Wifi,
        timg0.timer0,
        esp_hal::rng::Rng::new(peripherals.RNG),
        peripherals.RADIO_CLK,
        &clocks,
    )
    .unwrap();

    // initialize the maine executor with TIMG1.Timer0
    esp_hal_embassy::init(&clocks, em_timer0);

    // start softAP
    let wifi = peripherals.WIFI;
    let (wifi_interface, controller) = esp_wifi::wifi::new_with_mode(&wifi_init, wifi, WifiApDevice).unwrap();

    // ipv4 config
    let inet_config = embassy_net::Config::ipv4_static(
        StaticConfigV4 {
            address: Ipv4Cidr::new(embassy_net::Ipv4Address::new(192, 168, 5, 1), 24),
            gateway: Some(embassy_net::Ipv4Address::new(192, 168, 5, 1)),
            dns_servers: Default::default()
        }
    );

    // very random, very demure
    let seed = 1234;

    // init network stack
    let netstack = &*mk_static!(
        NetStack<WifiDevice<'_, WifiApDevice>>,
        NetStack::new(
            wifi_interface,
            inet_config,
            mk_static!(StackResources<3>, StackResources::<3>::new()),
            seed,
        )
    );

    spawner.spawn(new_connection(controller)).ok();
    spawner.spawn(net_task(netstack)).ok();

    // TODO: THIS IS WHERE I LEFT OFF FOR THE NIGHT
    // 
    // 
    // 
    // 
    // 
    // 

    loop {
        // wait for new connection
        
        // start new connection task

    }
}
