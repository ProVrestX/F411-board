# Modules

### stm32f411ceu6:
 - flash
 > spi1(pa5,6,7) + cs(pa4)
 - sd
 > spi1(pa5,6,7) + cs(pb9)
 - tft
 > spi1(pa5,6,7) + cs(pb10) + dc(pb12)
 - encoder
 > clk(pa1) + dt(pb4) + sw(pa0)
 - joystick
 > x(pb0) + y(pb1) + sw(pb8)
 - accel
 > i2c1(pb6,7) + int(pa15)
 - esp
 > uart2(pa2,3)
 - ext pins
 > uart1(pa9,10) + spi2(pb13,14,15) + i2c1(pb6,7)
 - other
 > led(pc13) + key(pa0) + buz(pa8) + dht(pb5)

### esp32c6:
 - stm
 > uart?()
 - oled
 > i2c?()
 - nrf24l01
 > spi?() + ce()
 - ir
 > rx() + tx()
 - encoder
 > clk() + dt() + sw()