Please include your answers to the questions below with your submission, entering into the space below each question
See [Mastering Markdown](https://guides.github.com/features/mastering-markdown/) for github markdown formatting if desired.

**1. How much current does a single LED draw when the output drive is set to "Strong" with the original code?**

Current draw when LED1 on = 4.71 mA
Current draw when LED1 off = 4.23 mA

Total current draw of LED1 = 0.48 mA

**2. After commenting out the standard output drive and uncommenting "Weak" drive, how much current does a single LED draw?**

Current draw when LED1 on = 4.86 mA
Current draw when LED1 off = 4.39 mA

Total current draw of LED1 = 0.47 mA

**3. Is there a meaningful difference in current between the answers for question 1 and 2? Please explain your answer, 
referencing the [Mainboard Schematic](https://www.silabs.com/documents/public/schematic-files/WSTK-Main-BRD4001A-A01-schematic.pdf) and [AEM Accuracy](https://www.silabs.com/documents/login/user-guides/ug279-brd4104a-user-guide.pdf) section of the user's guide where appropriate.**

There is no meaningful difference in current between the answers for question 1 and 2. A "strong" output drive should drive 10 mA current and a "weak" output drive should drive 1 mA current according to the reference manual. 
However, a common current-limiting resistance of 3k ohms is conected in series with both the LEDs. Therefore, with a common voltage source of 3.3 V, according to the ohms law, both the LED circuits would source the same current (around 1.1 mA) regardless of the drive strength.
Furthermore, LEDs have a very small current draw of the order observed in the answers of questions 1 and 2 and therefore, we get a lesser current draw from the LEDs than expected.  

**4. Using the Energy Profiler with "weak" drive LEDs, what is the average current and energy measured with only LED1 turning on in the main loop?**

Average Current = 4.71 mA
Average Energy  = 11.77 uWh

**5. Using the Energy Profiler with "weak" drive LEDs, what is the average current and energy measured with both LED1 and LED0 turning on in the main loop?**

Average Current = 4.87 mA
Average Energy  = 12.21 uWh
