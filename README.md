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

- 配置IWDG 喂狗间隔时间

- ![1714732759142](D:\wechat\WeChat Files\wxid_oncxri5u7fxe22\FileStorage\Temp\1714732759142.jpg)

  例：

  - 配置10s 
    - 看表格至少128分频
    - 10000  = ( 1 / 40 ) * 128 * RL
      - RL = 3125

```c

```

