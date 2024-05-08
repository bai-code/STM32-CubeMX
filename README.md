---

---

#### 3.USART 使用printf打印函数不需要开启中断

#### 4.USART简单的发送会导致丢失or发送卡死

- 解决：
  - 发送固定长度数据（用的少）

  - USART+DMA+IDLE空闲模式实现不定长度发送和接收

    - 发送不使用DMA，否则会导致只有第一次发送有效，或一直处于busy模式
    - 传输一半转运一次，满了再转运另外一半
    - 接收引脚   使用pull-up，circular模式

    ```c
    //main函数中调用一次
        HAL_UART_Receive_DMA(&huart2, (uint8_t *)RX2_DATA, RX2_LEN); //使能uart2，dma功能
        __HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);  //使能uart2， idle中断
        printf("接收数据\n");
    ```

    

```c
#define RX2_LEN 256 //数据长度
uint8_t RX2_DATA[RX2_LEN] = {0}; //数据存放
uint8_t RX2_OFFSET = 0;  //偏移量

void USART2_IRQHandler(void)
{
  /* USER CODE BEGIN USART2_IRQn 0 */

  /* USER CODE END USART2_IRQn 0 */
  	HAL_UART_IRQHandler(&huart2);
  /* USER CODE BEGIN USART2_IRQn 1 */
	USER_UART_IRQHandler(&huart2);   //调用一次
  /* USER CODE END USART2_IRQn 1 */
}
```

```c
// dma处理接收一半数据函数
void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart){
	if(huart->Instance == USART2){
		uint8_t len = RX2_LEN/2 - RX2_OFFSET;
		//printf("len= %d\n", len);
		HAL_UART_Transmit(huart, RX2_DATA + RX2_OFFSET,len, HAL_MAX_DELAY);
		RX2_OFFSET+=len;
		//printf("rx2_offset = %d\n", RX2_OFFSET);
	}
}
```

```c
//USART接受数据回调函数
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == USART2){
		uint8_t len = RX2_LEN - RX2_OFFSET;
		HAL_UART_Transmit(huart, RX2_DATA + RX2_OFFSET, len, HAL_MAX_DELAY);
		//printf("Complete length = %d", len);
		RX2_OFFSET = 0;
	}
}
```

```c
// 自定义处理空闲中断函数
void USER_UART_IRQHandler(UART_HandleTypeDef *huart){
	
	if(huart->Instance == USART2){
		
		if(__HAL_UART_GET_FLAG(huart, UART_FLAG_IDLE)!=RESET){ // 判断是否是空闲中断
			
			__HAL_UART_CLEAR_IDLEFLAG(huart); //清除空闲中断标志位（否则会一直进中断））
			uint8_t len = RX2_LEN  - __HAL_DMA_GET_COUNTER(&hdma_usart2_rx) - RX2_OFFSET;
			HAL_UART_Transmit(huart, RX2_DATA + RX2_OFFSET, len, HAL_MAX_DELAY);
			RX2_OFFSET += len;
			//printf("idle lenght=%d\n", len);  //测试使用
			
		}
	}
}
```

#### 5. IWDG

- LSI : 40KHz

- 配置IWDG 喂狗间隔时间

- ![1714732759142](D:/wechat/WeChat%20Files/wxid_oncxri5u7fxe22/FileStorage/Temp/1714732759142.jpg)

- 例：
  - 配置10s 
    - 看表格至少128分频
    - 10000  = ( 1 / 40 ) * 128 * RL
    - RL = 3125

- CubeMx 参数含义
  - psc  		     :128
  - dc_reload_value : 3125
  - 得到最大10s喂狗时间

#### 6.WWDG

- 配置WWDG喂狗时间

  ![image-20240503203406651](../../AppData/Roaming/Typora/typora-user-images/image-20240503203406651.png)

- 例 ：配置 超时时间 50ms（最长时间）， 窗口时间20ms（最短时间）

  - 依最大超时间，WDGTB = 3
  - 超时时间：50ms = 1 / 36000 * 4096 * 2^3 *  ( T[5:0]+1 )
    - T[5:0] + 1 ≈ 54.9
      - T[5:0]  = 53.9
  - 窗口时间：20ms = 1 / 36000 * 4096 * 2^3 * ( T[5:0] - W[5:0] )
    - T[5:0] - W[5:0] ≈ 21.9
      - W[5:0] = 32

  

- CubeMx 参数含义

  - prc ：                            2^WDGTB   // 2的次方  

  - window value ： 	64 - 127       // 0x40  -  0x7f

  - downcounter value :  64 - 127

    

  - 例： psc  : 8    

    - w_value :  0x70 
    - dc_value : 0x7f

  - 喂狗窗口：PCLK1 / 8 / 4096  =  1099Hz

    - ​	    ( 0x7f - 0x3f ) / 1099 =  58ms
    - ​	    ( 0x7f - 0x70 ) / 1099 = 14ms

- 窗口看门狗中断（达到延时复位作用）

  ```c
  // 按窗口看门狗上限时间为一个周期执行一次
  // 例：50ms
  void HAL_WWDG_EarlyWakeupCallback(WWDG_HandleTypeDef *hwwdg)
  {
  	static uint32_t wwdg_count = 0;
  	wwdg_count ++;
  	// 例：当4s内按键按下
  //	if(key_down){
  //		wwdg_count = 0;  // 4s从新开始计时
  //	}
  	
  	if(wwdg_count <=80){   //  50*80  = 4s 
  		HAL_WWDG_Refresh(hwwdg);  //喂狗
  	}else{
  		// 超过4s 窗口看门狗复位
  	}
  }
  ```

  

#### 7.RTC

- 使能LSE（32.768）
-  CubeMX 配置
  - Date formate ： BCD data formate

```c
// 测试实时时钟
void RTC_Test(void){
	RTC_TimeTypeDef Time = {0};
	RTC_DateTypeDef Date = {0};
	HAL_RTC_GetTime(&hrtc, &Time, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &Date, RTC_FORMAT_BIN);
	printf("%04d-%02d-%02d  %02d:%02d:%02d \n", 2000+Date.Year, Date.Month, 							Date.Date, Time.Hours, Time.Minutes, Time.Seconds);
	HAL_Delay(1000);
}
```

- 问题： 复位时钟年月日时分秒都会初始化

  - 解决： MX_RTC_Init函数，注释部分代码, 会导致重新设置时间值

  - 该方法会导致==年月日不被设置， 时分秒复位不会初始化==

  - ```c
      /** Initialize RTC and set the Time and Date
      */
    //  sTime.Hours = 0x22;
    //  sTime.Minutes = 0x12;
    //  sTime.Seconds = 0x30;
    
    //  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
    //  {
    //    Error_Handler();
    //  }
    //  DateToUpdate.WeekDay = RTC_WEEKDAY_MONDAY;
    //  DateToUpdate.Month = RTC_MONTH_MAY;
    //  DateToUpdate.Date = 0x5;   //下载后时间变成2000-01-01， 年月日无法被设置
    //  DateToUpdate.Year = 0x23;
    
    //  if (HAL_RTC_SetDate(&hrtc, &DateToUpdate, RTC_FORMAT_BCD) != HAL_OK)
    //  {
    //    Error_Handler();
    //  }
    ```

  - 解决复位保存年月日时分秒

    - ==使用备份寄存器，保存一个标志，如果该位置不存在某个定值，则会初始化==

    - ```c
        /* USER CODE BEGIN RTC_Init 1 */
        	
        	__HAL_RCC_BKP_CLK_ENABLE(); // 使能BKP备份寄存器时钟
        	__HAL_RCC_PWR_CLK_ENABLE(); // 使能PWR备份寄存器电源
        	
        /* USER CODE END RTC_Init 1 */
      ```

      ```c
       --》if(HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1)!= 0xA5A5 )   // 判断寄存器是否存在存储的固定值
      {	
        /* USER CODE END Check_RTC_BKUP */
      
        /** Initialize RTC and set the Time and Date
        */
        sTime.Hours = 0x22;
        sTime.Minutes = 0x12;
        sTime.Seconds = 0x30;
      
        if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
        {
          Error_Handler();
        }
        DateToUpdate.WeekDay = RTC_WEEKDAY_MONDAY;
        DateToUpdate.Month = RTC_MONTH_MAY;
        DateToUpdate.Date = 0x5;
        DateToUpdate.Year = 0x23;
      
        if (HAL_RTC_SetDate(&hrtc, &DateToUpdate, RTC_FORMAT_BCD) != HAL_OK)
        {
          Error_Handler();
        }
        /* USER CODE BEGIN RTC_Init 2 */
      --》	HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, 0xA5A5);   //进判断后，把固定值存放
      }
      ```

      - 注解： 使能备份寄存器，判断备份寄存器中的某个位置是否存放某个值，如果存放，则不是第一次进入，无需重新设置时间，否则，从新设置时间（上述代码还存在bug，思路已明确）


#### 8.PWR

##### Sleep mode

- 各种模式都会保持该模式关闭前的引脚状态，而关闭模式为了省电，所以关闭前都应该关闭其他耗能设备

  ```c
  while (1)
    {
  		LED_SwitchStatus();
  		HAL_Delay(2000);
  		LED_SwitchStatus();
  		printf("system running\n");
  		
  		LED_SwitchStatus();
  		HAL_SuspendTick();  //暂停嘀嗒定时器
  		printf("system sleeping\n");
      // params1 search “sleep mode“
  		HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON ,PWR_SLEEPENTRY_WFI);
  		
  		printf("system wakeup\n");
  		HAL_ResumeTick();  // 恢复滴答定时器
  		// 不恢复嘀嗒定时器，将不再执行while循环
      /* USER CODE END WHILE */
    }
  ```

##### Stop mode 

- 停止模式开启后，关闭HSI 、HSE， 时钟使用 LSI (8M) ， 恢复后仍然使用LSI，需重新配置时钟

- 唤醒方式：==WKUP引脚的上升沿、RTC闹钟事件的上升沿、NRST引脚上外部复位、IWDG复位退出待机模式==

    ```c
    while (1)
    {
    		LED_SwitchStatus();
    		HAL_Delay(2000);
    		LED_SwitchStatus();
    		printf("system running\n");
    		
    		LED_SwitchStatus();
    		HAL_SuspendTick();  //暂停嘀嗒定时器
    		printf("system stop\n");
    		HAL_PWR_EnterSTOPMode(PWR_MAINREGULATOR_ON ,PWR_STOPENTRY_WFI);  // 开启停止模式
    		
    -->		SystemClock_Config();   // 从新初始化时钟
    		
    		HAL_ResumeTick();  // 恢复滴答定时器
    		printf("system wakeup\n");
    		
    		// 不恢复嘀嗒定时器，将不再执行while循环
    		
    		printf("sysclk = %d hclk = %d pclk:%d pclk2:%d\n source:%d \n ", HAL_RCC_GetSysClockFreq(),HAL_RCC_GetHCLKFreq(),
    		HAL_RCC_GetPCLK1Freq(), HAL_RCC_GetPCLK2Freq(), __HAL_RCC_GET_SYSCLK_SOURCE());
        /* USER CODE END WHILE */
      }
    ```

    ##### Standby mode

    ```c
    int main(void){
    	
    	HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);  //开启wakeup 引脚唤醒功能
        while(1){
            LED_SwitchStatus();
    		HAL_Delay(2000);
    		LED_SwitchStatus();
    		printf("system running\n");
    		
    		LED_SwitchStatus();
    		HAL_SuspendTick();  //暂停嘀嗒定时器
    		printf("system standby\n");
    		
    		
    		SystemClock_Config();   // 从新初始化时钟
    		
    		HAL_ResumeTick();  // 恢复滴答定时器
    		printf("system wakeup\n");
    		
    		// 不恢复嘀嗒定时器，将不再执行while循环
    		
    		printf("sysclk = %d hclk = %d pclk:%d pclk2:%d\n source:%d \n ", 				HAL_RCC_GetSysClockFreq(),HAL_RCC_GetHCLKFreq(),
    		HAL_RCC_GetPCLK1Freq(), HAL_RCC_GetPCLK2Freq(), __HAL_RCC_GET_SYSCLK_SOURCE());
    -->     __HAL_RCC_PWR_CLK_ENABLE();     //使能PWR时钟
    -->    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);      //清除Wake_UP标志
    -->		HAL_PWR_EnterSTANDBYMode();  // 待机（关机）模式	
            
        }
    }
    ```

#### 9.ADC

- 单次转换，不连续，不扫描

  ```c
  while (1)
    {
  		uint32_t adc_value = 0;
  		//for(uint8_t i = 0; i<10; i++){
  			HAL_ADC_Start(&hadc1);  // 启动adc，启动一次工作一次
  			if(HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK){ // 等待adc转换结束， 100：timeout时间
  				adc_value = HAL_ADC_GetValue(&hadc1);
  				//adc_value += HAL_ADC_GetValue(&hadc1);  //10次求平均值数值更准确
  			}
  		//}
  		//adc_value = adc_value /10;
  		//0    ----- 0v
  		//4096 ------3.3v
  		//val = x/4095*3.3
  		
  		float val = ((float)adc_value/4095*3.3);
  		printf("adc_value: %.2f \n", val);
  		HAL_Delay(500);
  	}
  ```

- 测量stm32  MCU温度

  ```c
   while (1)
    {
  		uint32_t adc_value = 0;
  		//for(uint8_t i = 0; i<10; i++){
  			HAL_ADC_Start(&hadc1);  // 启动adc，启动一次工作一次
  			if(HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK){ // 等待adc转换结束， 100：timeout时间
  				adc_value = HAL_ADC_GetValue(&hadc1);
  				//adc_value += HAL_ADC_GetValue(&hadc1);  //10次求平均值数值更准确
  			}
  		//}
  		//adc_value = adc_value /10;
  		//0    ----- 0v
  		//4095 ------3.3v
  		//val = x/4095*3.3
  		
  -->		float cpu_temper = (1.43 - adc_value*3.3/4095)/0.0043 + 25; //查看电气特性，
  		//float val = (adc_value*3.3/4095);
  		printf("adc_value: %.2f \n", cpu_temper);
  		HAL_Delay(500);
   }
  ```

  
