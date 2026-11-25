# STM32 串口收发实验

基于 STM32F103C8T6 的 USART1 串口通信实验，配套江协科技 STM32 入门教程（第 9 章）。

## 功能

- USART1 收发，波特率 **9600，8 数据位，1 停止位，无校验**
- 上电发送 `A666`
- 串口中断接收，收到的字节在 OLED 上以十六进制显示
- `printf` 重定向到串口（MicroLIB + `fputc`）

## 硬件接线

| STM32 引脚 | 功能 | 接到 |
|---|---|---|
| PA9 | USART1_TX | USB-TTL 模块的 **RXD** |
| PA10 | USART1_RX | USB-TTL 模块的 **TXD** |
| GND | 地 | USB-TTL 模块的 **GND**（必须共地） |
| PB8 / PB9 | 软件 I2C | OLED 显示屏 |

> 注意：TX/RX 必须交叉；VCC 可不接，用板子自身 USB 供电更稳。

## 代码结构

```
User/main.c          主逻辑：初始化、发送、接收显示
Hardware/usart.c     USART1 初始化、发送、中断接收
Hardware/oled.c      OLED 显示（软件 I2C）
Library/             STM32F10x 标准外设库
Start/               启动文件与系统初始化
```

## 关键实现

**串口初始化（PA9 复用推挽输出，PA10 上拉输入）**

```c
GPIO_InitTypeDef GPIO_InitStructure;

GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
GPIO_Init(GPIOA, &GPIO_InitStructure);

GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
GPIO_Init(GPIOA, &GPIO_InitStructure);
```

**发送并等待发送寄存器空**

```c
void send(uint8_t byte)
{
    USART_SendData(USART1, byte);
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
}
```

**printf 重定向**

```c
int fputc(int ch, FILE *f)
{
    send(ch);
    return ch;
}
```
需要在 Keil 中勾选 **Use MicroLIB**。

## 调试记录：一个“能收不能发”的坑

**现象**：串口能接收（OLED 正常显示收到的字节），但电脑收不到任何数据。

**排查过程**：

1. 发送函数执行后 `USART1->SR` 显示 TXE/TC 已置位 → USART 内部认为发送完成
2. 读 `GPIOA->CRH` 发现 **PA9 = 0x8（输入上拉）**，而不是期望的 `0xB`（复用推挽输出）
3. 反汇编确认：第二次 `GPIO_Init` 传入的结构体指针指向未初始化内存

**根因**：初始化时声明了两个 `GPIO_InitTypeDef`，但第二组赋值写到了第一个变量上：

```c
GPIO_InitTypeDef GPIO_InitStructure2;          // 从未赋值
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  // 写错变量名
GPIO_Init(GPIOA, &GPIO_InitStructure2);        // 传入未初始化结构体
```

未初始化的栈数据把 PA9 覆盖成了输入模式，导致 TX 引脚不出信号。

**修复**：只用一个结构体，改 Pin/Mode 后复用。

**教训**：外设不工作时先读寄存器（CRH/CRL/SR），比反复换线快得多。

## 编译环境

- Keil MDK 5（ARMCC V5.06）
- STM32F10x 标准外设库
- 目标芯片：STM32F103C8
- 需勾选 **Use MicroLIB**（printf 重定向）
