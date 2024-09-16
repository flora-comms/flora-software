#![no_std]
#![no_main]


use core::ptr::addr_of_mut;

use embassy_executor::Spawner;
use embassy_time::{Duration, Ticker};
use embassy_sync::{blocking_mutex::raw::CriticalSectionRawMutex, signal::Signal};
use esp_backtrace as _;
use esp_hal::{
    clock::ClockControl, cpu_control::{CpuControl, Stack}, get_core, gpio::{GpioPin, Io, Level, Output}, peripherals::Peripherals, system::SystemControl, timer::timg::TimerGroup
};
use esp_hal_embassy::Executor;
use esp_println::println;
use static_cell::{self, StaticCell};

static mut APP_CORE_STACK: Stack<8192> = Stack::new();

// Says hello every 2 seconds
#[embassy_executor::task]
async fn blink_2sec (
    mut led: Output<'static, GpioPin<37>>, 
    cntl_signal: &'static Signal<CriticalSectionRawMutex, bool>,
) {
    println!("Hello from core {:?}", get_core() as usize);
    loop {
        if cntl_signal.wait().await {
            println!("LED ON signal recieved on core {:?}. Turning LED ON...", get_core() as usize);
            led.set_high();
        } else {
            println!("LED OFF signal recieved on core {:?}. Turning LED ON...", get_core() as usize);
            led.set_low();
        }
    }
}

#[esp_hal_embassy::main]
async fn main(_spawner: Spawner) -> ! {
    let peripherals = Peripherals::take();
    let system = SystemControl::new(peripherals.SYSTEM);
    let io = Io::new(peripherals.GPIO, peripherals.IO_MUX);
    let clocks = ClockControl::max(system.clock_control).freeze();

    esp_println::logger::init_logger_from_env();

    // init timer group 0 for wifi and timer group 1 for the executor
    let timg0 = TimerGroup::new(peripherals.TIMG0, &clocks);  
    let timg1 = TimerGroup::new(peripherals.TIMG1, &clocks);

    // create a hardware timer for use by embassy
    let em_timer0 = timg1.timer0;
    // wifi initialization
    let _init = esp_wifi::initialize(
        esp_wifi::EspWifiInitFor::Wifi,
        timg0.timer0,
        esp_hal::rng::Rng::new(peripherals.RNG),
        peripherals.RADIO_CLK,
        &clocks,
    )
    .unwrap();

    // initialize the maine executor with TIMG1.Timer0
    esp_hal_embassy::init(&clocks, em_timer0);

    // create led pin
    let led = Output::new(io.pins.gpio37, Level::High);

    // create a closure to run the app core task
    let app_task = move |led: Output<'static, GpioPin<37>>, cntl_signal: &'static Signal<CriticalSectionRawMutex, bool>| {
        static EXECUTOR: StaticCell<Executor> = StaticCell::new();
        let executor = EXECUTOR.init(Executor::new());
        executor.run(|spawner| {
            spawner.spawn(blink_2sec(led, cntl_signal)).ok();
        });
    };

    // cpu app core stuff
    // create an LED control signal
    // This is the control signal. It is a statically allocated critical section mutex, so it disables interrupts. The mutex contains a bool indicating the state of the LED
    static LED_CNTL: StaticCell<Signal<CriticalSectionRawMutex, bool>> = StaticCell::new();
    // This initializes the led control signal and actually places the signal in the static cell. The static cell is dereferenced and initialized, with the mutex type being infered. Then a reference to it is created as led_cntl_signal
    let led_cntl_signal = &*LED_CNTL.init(Signal::new());
    // create a cpu control object to assign tasks to the different cores
    let mut cpu_control = CpuControl::new(peripherals.CPU_CTRL);
    // create a guard around the app core
    let _guard = cpu_control
        .start_app_core(unsafe { &mut *addr_of_mut!(APP_CORE_STACK) }, move || { app_task(led, led_cntl_signal) })
        .unwrap();


    // create a ticker to control the timing of the led.
    let mut ticker = Ticker::every(Duration::from_millis(500));
    println!("Starting led control on main core - {:?}", get_core() as usize);

    loop {
        println!("Sending LED ON signal from core {:?}", get_core() as usize);
        led_cntl_signal.signal(true);
        ticker.next().await;

        println!("Sending LED OFF signal from core {:?}", get_core() as usize);
        led_cntl_signal.signal(false);
        ticker.next().await;
    }
}
