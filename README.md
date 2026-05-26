<h1>BCA143 Laboratory Activity 10</h1>
<h2>Signal Generator with Precision Timing and Buffering</h2>

<p>
  <strong>Course:</strong> BCA143 Firmware Programming<br>
  <strong>Board:</strong> RT-Thread RT-Spark Development Board (STM32F407)<br>
  <strong>Tools:</strong> STM32CubeIDE, STM32CubeMX, Oscilloscope / Logic Analyzer
</p>

<hr>

<h2>About This Project</h2>

<p>
  This project demonstrates waveform generation on the RT-Spark STM32F407
  development board using a periodic timer interrupt and a FIFO queue buffer.
  The firmware produces sine, square, and ramp waveforms through the DAC, and
  compares two playback approaches — busy-wait and interrupt-driven — to show
  how a queue buffer protects output timing even when the main program is
  temporarily blocked by a button press.
</p>

<hr>

<h2>Project Documentation</h2>

<h3>1. RT-Spark Board Wiring</h3>
<p>
  RT-Thread RT-Spark Development Board wired for Lab 10, with oscilloscope probes
  connected to the DAC output (PA4) and SW1 button (PB2) for signal monitoring.
</p>
<p>
<p align="center">
<img src="https://github.com/user-attachments/assets/f5dff73b-c1e1-48d6-a6c7-207d246e4359" alt="RT-Spark board wiring" width="400"/>
</p>

<h3>2. Oscilloscope Screen (Busy-Wait Mode)</h3>
<p>
  25 Hz sine wave output captured during busy-wait playback. The waveform
  immediately flatlines upon pressing SW1, demonstrating how blocking the main
  loop directly halts signal generation with no buffering.
</p>
<p>
<p align="center">
<img src="https://github.com/user-attachments/assets/361cc6b7-e3a8-416c-a1e6-a91b1d0f8e8f" alt="Oscilloscope busy-wait mode" width="400"/>
</p>

<h3>3. Oscilloscope Screen (Interrupt-Driven Mode)</h3>
<p>
  25 Hz sine wave output captured during interrupt-driven playback. The waveform
  continues for approximately 10 ms after pressing SW1 before stopping, showing
  the queue buffer absorbing the delay before underflow occurs.
</p>
<p>
<p align="center">
<img src="https://github.com/user-attachments/assets/267696d3-544f-4b3f-9f23-28f6b6abf65b" alt="Oscilloscope interrupt-driven mode" width="400"/>
</p>

<h3>4. STM32CubeMX Clock Configuration</h3>
<p>
  System clock configured to 168 MHz using HSI as the PLL source via STM32CubeMX.
  This clock frequency is referenced in delay.c as CLK_FREQ for accurate
  microsecond and millisecond timing calculations.
</p>
<p>
<p align="center">
<img src="https://github.com/user-attachments/assets/ead748e0-2d8f-4a1b-8cc8-44208d9cb8d4" alt="STM32CubeMX clock configuration" width="800"/>
</p>

<h3>5. STM32CubeMX TIM2 Configuration</h3>
<p>
  TIM2 configured with Internal Clock source and Channel 1 set to Output Compare
  No Output mode. The timer interrupt is enabled in NVIC settings, and its
  period is set dynamically at runtime by timer_init() in platform.c.
</p>
<p>
<p align="center">
<img src="https://github.com/user-attachments/assets/89f57163-4424-4c17-bb9d-a8ce9c4fa62c" alt="STM32CubeMX TIM2 configuration" width="800"/>
</p>

<h3>6. STM32CubeMX DAC Configuration</h3>
<p>
  DAC Channel 1 configured with no trigger and output buffer enabled. The DAC
  output on PA4 serves as the analog waveform output, updated each timer ISR
  firing via dac_set() with 12-bit resolution (0–4095).
</p>
<p>
<p align="center">
<img src="https://github.com/user-attachments/assets/fd3cb2cd-7134-497c-9eea-0379ccd542ce" alt="STM32CubeMX DAC configuration" width="800"/>
</p>

<h3>7. STM32CubeMX GPIO Configuration</h3>
<p>
  GPIO configuration showing PF8, PF9, and PF10 as push-pull outputs for the
  onboard RGB LED, and PB2 configured as a pull-up input for the SW1 button
  used to simulate processing interference.
</p>
<p>
<p align="center">
<img src="https://github.com/user-attachments/assets/ea3fac77-0710-4f59-ba43-5cb7675e94f2" alt="STM32CubeMX GPIO configuration" width="800"/>
</p>

<h3>8. main.c Preview</h3>
<p>
  Main program configured for Section 4.3 interrupt-driven mode, generating a
  25 Hz sine wave with a 40,000 µs period. The BUSY_WAIT macro is commented out
  to select interrupt-driven playback using the queue-buffered
  tone_play_with_interrupt() function.
</p>
<p>
<p align="center">
<img src="https://github.com/user-attachments/assets/2ca36d92-9521-4401-a902-5c3f60cdf0e0" alt="main.c code preview" width="800"/>
</p>

<hr>

<h2>Laboratory Questions and Answers</h2>

<h3>Q1. Busy-Wait: How long after pressing the switch does the output stop?</h3>

<p>
  The output stops <strong>immediately</strong> — within just a few microseconds
  of pressing the button.
</p>

<p>
  This happens because in busy-wait mode, the MCU is doing everything itself one
  step at a time. When you press the button, the code gets stuck waiting for you
  to release it. Since nothing else is running in the background, the DAC stops
  updating and the waveform freezes right away.
</p>

<hr>

<h3>Q2. Interrupt-Driven: How long after pressing the switch does the output stop changing?</h3>

<p>
  The output keeps going for about <strong>10 milliseconds</strong> after you
  press the button, then stops.
</p>

<p>
  Here's why: the main code pre-loads samples into a queue — like a waiting line
  of 16 values. Even when the main code is blocked by the button press, a
  separate timer interrupt keeps pulling values out of the queue and sending
  them to the DAC.
</p>

<p>
  <code>16 samples × 625 µs per sample = 10 ms</code>
</p>

<p>
  So the queue gives you 10 ms of extra playtime before it runs empty and the
  output finally stops.
</p>

<hr>

<h3>Q3. How long does the buffer take to empty — when does the LED turn red?</h3>

<p>
  The LED turns red about <strong>10 ms</strong> after pressing the button.
</p>

<p>
  Every 625 µs, the timer ISR takes one sample out of the queue. With 16 slots:
</p>

<p>
  <code>16 × 625 µs = 10 ms to drain completely</code>
</p>

<p>
  When the last sample is removed, the ISR detects an empty queue and turns the
  LED red to signal underflow. You can measure this exactly on the logic
  analyzer by watching the gap between the SW1 falling edge and the red LED
  signal going high.
</p>

<hr>

<h3>Q4. How long does the buffer take to fill — when does the LED turn green? How many CPU cycles per sample?</h3>

<p>
  The buffer fills almost <strong>instantly</strong> — within just a few
  microseconds of releasing the button. The LED turns green almost right away.
</p>

<p>
  Loading one sample takes roughly <strong>100 ns</strong>, which is about
  <strong>17 CPU clock cycles</strong> at 168 MHz:
</p>

<p>
  <code>1 cycle = 1 ÷ 168,000,000 ≈ 5.95 ns</code><br>
  <code>100 ns ÷ 5.95 ns ≈ 17 cycles</code>
</p>

<p>
  The main code fills samples hundreds of times faster than the ISR drains them,
  so the queue jumps to full almost immediately after the button is released.
</p>

<p>
  <strong>Why is the queue sometimes not full after it has been filled?</strong>
  Because the timer ISR keeps draining samples at the same time the main code is
  trying to fill. As soon as the queue hits full (green LED), the ISR pops one
  out — the queue drops just below full and the LED flips to blue. Then the
  main code adds one back, and it goes green again. This back-and-forth race
  between the producer (main code) and consumer (ISR) causes the LED to flicker
  between green and blue near the top of the queue.
</p>

<hr>

<h2>Files in This Repository</h2>

<ul>
  <li><code>main.c</code>: main application code, waveform selection, HAL peripheral initialization</li>
  <li><code>tone.c / tone.h</code>: waveform generation, busy-wait and interrupt-driven playback, RGB LED queue monitoring</li>
  <li><code>queue.c / queue.h</code>: circular FIFO buffer implementation</li>
  <li><code>platform.c / platform.h</code>: HAL abstraction layer for DAC, TIM2, and GPIO</li>
  <li><code>delay.c / delay.h</code>: microsecond and millisecond delay functions</li>
</ul>

<hr>

<h2>How to Build</h2>

<ol>
  <li>Open <strong>STM32CubeIDE</strong></li>
  <li>Import the project folder</li>
  <li>Click the hammer icon to <strong>Build</strong></li>
  <li>Click the play icon to <strong>Flash</strong> to the RT-Spark board</li>
  <li>Connect an oscilloscope or logic analyzer to <strong>PA4 (DAC OUT)</strong> and <strong>PB2 (SW1)</strong></li>
  <li>Observe the 25 Hz sine wave and press SW1 to test busy-wait vs interrupt behavior</li>
  <li>Watch the RGB LED: <strong>green</strong> = buffer full, <strong>blue</strong> = partially filled, <strong>red</strong> = buffer empty</li>
</ol>

<hr>

<p>
  <em>Prepared by Jhanaloden Dipantar</em><br>
  <em>May 2026</em><br>
  <em>BCA143 Firmware Programming</em><br>
  <em>Mindanao State University - Iligan Institute of Technology</em>
</p>
