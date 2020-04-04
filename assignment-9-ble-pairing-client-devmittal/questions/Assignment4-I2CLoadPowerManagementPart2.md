Please include your answers to the questions below with your submission, entering into the space below each question
See [Mastering Markdown](https://guides.github.com/features/mastering-markdown/) for github markdown formatting if desired.

*Be sure to take measurements in the "Default" configuration of the profiler to ensure your logging logic is not impacting current/time measurements.*

*Please include screenshots of the profiler window detailing each current measurement captured.  See [Shared document](https://docs.google.com/document/d/1Ro9G2Nsr_ZXDhBYJ6YyF9CPivb--6UjhHRmVhDGySag/edit?usp=sharing) for instructions.* 

1. What is the average current per period?
   Answer: 4.41 uA
   <br>Screenshot:  
   ![Avg_current_per_period](https://github.com/CU-ECEN-5823/assignment-4-devmittal/blob/master/Screenshots/Assignment-4/Avg_current_per_period.PNG)  

2. What is the average current when the Si7021 is Load Power Management OFF?
   Answer: 1.21 uA
   <br>Screenshot:  
   ![Avg_current_LPM_Off](https://github.com/CU-ECEN-5823/assignment-4-devmittal/blob/master/Screenshots/Assignment-4/Avg_current_LPM_Off.PNG)  

3. What is the average current when the Si7021 is Load Power Management ON?
   Answer: 98.54 uA
   <br>Screenshot:  
   ![Avg_current_LPM_On](https://github.com/CU-ECEN-5823/assignment-4-devmittal/blob/master/Screenshots/Assignment-4/Avg_current_LPM_On.PNG)  

4. How long is the Si7021 Load Power Management ON for 1 temperature reading?
   Answer: 92.20 ms
   <br>Screenshot:  
   ![duration_lpm_on](https://github.com/CU-ECEN-5823/assignment-4-devmittal/blob/master/Screenshots/Assignment-4/duration_lpm_on.PNG)  

5. What is the total operating time of your design for assignment 4 in hours assuming a 1000mAh supply?

	time = 1000 / (avg current per period)
		 = 1000/(4.41 * 10^(-3))
		 = 226757.369 hours

6. How has the power consumption performance of your design changed since the previous assignment?
	The power consumption has reduced as compared to that from the previous assignment. All 3 readings - the average current per period, the average current when load management is off and average current when load management is on has reduced 
	due to the use of non blocking i2c routines and non blocking timer delays which allows the system to sleep in the deepest state possible when doing no work to conserve as much power as possible.

7. Describe how you have tested your code to ensure you are sleeping in EM1 mode during I2C transfers.
	The sleep block begin and sleep block end, around the i2c writes and read, with a parameter of sleepEM2 ensures that the system does not sleep below EM1 and stays in EM1 mode while the I2C transfers occur. Furthermore, on observing the energy
	profiler and the screenshots provided above, we see that the current consumed during the I2C transfers (i.e. after the 80ms delay and then again after the 10ms delay), is around 4-5 mA indicating EM1 mode. This was further verified by obtaining the return status of SLEEP_sleep 
	function to check what mode the system is currently sleeping in.
	   