# STM32F407 Network OTA Update

基于STM32F407的网络固件在线升级(OTA)

## 项目简介

本项目实现了STM32F407的网络固件在线升级功能，支持通过以太网更新固件。
PC端的TCP服务程序位于./PYTHON/tcp_server.py ，使用python运行，如缺少相关依赖请自行安装
PC端的TCP服务程序实现了设置TCP服务的IP和端口的功能，以及传输固件文件的功能

STM32端基于LwIP实现了一个TCP客户端，并且可以接收固件文件，固件文件存放在外部SPI Flash。
注意，这个固件升级功能需要搭配bootloader使用，bootloader的仓库位于：
https://github.com/SloftyLiu/stm32f407_bootloader_spiflash

## 硬件平台
本程序兼容正点原子F407开发板
