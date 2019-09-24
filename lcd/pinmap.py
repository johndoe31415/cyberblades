#!/usr/bin/python3
#	pibeatsaber - Beat Saber historian application that tracks players
#	Copyright (C) 2019-2019 Johannes Bauer
#
#	This file is part of pibeatsaber.
#
#	pibeatsaber is free software; you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation; this program is ONLY licensed under
#	version 3 of the License, later versions are explicitly excluded.
#
#	pibeatsaber is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with this program.  If not, see <https://www.gnu.org/licenses/>.
#
#	Johannes Bauer <JohannesBauer@gmx.de>

rpi_pinout = {
	1:	"3.3V",
	2:	"5V",
	3:	"GPIO2 (SDA1)",
	4:	"5V",
	5:	"GPIO3 (SCL1)",
	6:	"GND",
	7:	"GPIO4 (GPIO_GCLK)",
	8:	"GPIO14 (TXD0)",
	9:	"GND",
	10:	"GPIO15 (RXD0)",
	11:	"GPIO17 (GPIO_GEN0)",
	12:	"GPIO18 (GPIO_GEN1)",
	13:	"GPIO27 (GPIO_GEN2)",
	14:	"GND",
	15:	"GPIO22 (GPIO_GEN3)",
	16:	"GPIO23",
	17:	"3.3V",
	18:	"GPIO24",
	19:	"GPIO10 (SPI_MOSI)",
	20:	"GND",
	21:	"GPIO9 (SPI_MISO)",
	22:	"GPIO25 (GPIO_GEN6)",
	23:	"GPIO11 (SPI_SCLK)",
	24:	"GPIO8 (SPI_CE0_N)",
	25:	"GND",
	26:	"GPIO7 (SPI_CE1_N)",
	27:	"ID_SD",
	28:	"ID_SC",
	29:	"GPIO5",
	30:	"GND",
	31:	"GPIO6",
	32:	"GPIO12",
	33:	"GPIO13",
	34:	"GND",
	35:	"GPIO19",
	36:	"GPIO16",
	37:	"GPIO26",
	38:	"GPIO20",
	39:	"GND",
	40:	"GPIO21",
}

display_pinout = {
	1:	"3.3V",
	2:	"5V",
	3:	"SDA",
	4:	"5V",
	5:	"SCL",
	6:	"GND",
	7:	"P7",
	8:	"TX",
	9:	"GND",
	10:	"RX",
	11:	"P0",
	12:	"P1",
	13:	"P2",
	14:	"GND",
	15:	"P3",
	16:	"P4",
	17:	"3.3V",
	18:	"P5",
	19:	"MOSI",
	20:	"GND",
	21:	"MISO",
	22:	"P6",
	23:	"SCK",
	24:	"CE0 (LCD CS)",
	25:	"GND",
	26:	"CE1 (Touch CS)",
}

#if True:
#	display_pinout[11] += " (Touch IRQ)"
#	display_pinout[18] += " (Instruction/Data RS)"
#	display_pinout[22] += " (RESET)"
#else:
#	display_pinout[11] += " (Backlight)"
#	display_pinout[22] += " (Touch IRQ)"

display_pinout[11] += " (Touch IRQ)"
display_pinout[12] += " (Button Top K1)"
display_pinout[16] += " (Button Middle K2)"
display_pinout[18] += " (Button Low K3)"

#for i in range(20):
#	pin_l = 2 * i + 1
#	pin_r = pin_l + 1
#	print("%30s %-2d %2d %-30s" % (rpi_pinout[pin_l], pin_l, pin_r, rpi_pinout[pin_r]))

for (display_pin, display_name) in sorted(display_pinout.items()):
	rpi_pin = display_pin
	rpi_name = rpi_pinout[rpi_pin]
	print("%30s = [%2d | %2d] = %-30s" % (display_name, display_pin, rpi_pin, rpi_name))
