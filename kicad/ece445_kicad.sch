EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr USLetter 11000 8500
encoding utf-8
Sheet 1 1
Title "Project Template"
Date ""
Rev "0.0"
Comp "Team: #1"
Comment1 "Designed By: NAME"
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L ece445_Connector_Generic:Conn_01x02 J1
U 1 1 5E756461
P 3000 2300
F 0 "J1" H 2918 1975 50  0000 C CNN
F 1 "Conn_01x02" H 2918 2066 50  0000 C CNN
F 2 "ece445_Connector_PinHeader_2.54mm:PinHeader_1x02_P2.54mm_Vertical" H 3000 2300 50  0001 C CNN
F 3 "~" H 3000 2300 50  0001 C CNN
	1    3000 2300
	-1   0    0    1   
$EndComp
$Comp
L ece445_intro:TPS799 U1
U 1 1 5E75683C
P 4150 2250
F 0 "U1" H 4150 2525 50  0000 C CNN
F 1 "TPS799" H 4150 2434 50  0000 C CNN
F 2 "ece445_intro:TPS799" H 4150 2250 50  0001 C CNN
F 3 "http://www.ti.com/lit/ds/symlink/tps799.pdf" H 4150 2250 50  0001 C CNN
	1    4150 2250
	1    0    0    -1  
$EndComp
$Comp
L ece445_device:R R1
U 1 1 5E756E7C
P 4800 2200
F 0 "R1" V 4593 2200 50  0000 C CNN
F 1 "100 Ohms" V 4684 2200 50  0000 C CNN
F 2 "ece445_Resistor_THT:R_Axial_DIN0204_L3.6mm_D1.6mm_P5.08mm_Horizontal" V 4730 2200 50  0001 C CNN
F 3 "~" H 4800 2200 50  0001 C CNN
	1    4800 2200
	0    1    1    0   
$EndComp
$Comp
L ece445_device:LED D1
U 1 1 5E757210
P 4950 2650
F 0 "D1" V 4989 2533 50  0000 R CNN
F 1 "LED" V 4898 2533 50  0000 R CNN
F 2 "ece445_LED_THT:LED_Rectangular_W5.0mm_H2.0mm" H 4950 2650 50  0001 C CNN
F 3 "~" H 4950 2650 50  0001 C CNN
	1    4950 2650
	0    -1   -1   0   
$EndComp
Wire Wire Line
	3200 2200 3500 2200
Wire Wire Line
	4950 2200 4950 2500
Wire Wire Line
	3200 2300 3200 2800
Wire Wire Line
	3200 2800 3500 2800
Connection ~ 4150 2800
Wire Wire Line
	4150 2650 4150 2800
Wire Wire Line
	3850 2350 3850 2200
Connection ~ 3850 2200
$Comp
L ece445_power:+5V #PWR0101
U 1 1 5E761ADF
P 3500 2200
F 0 "#PWR0101" H 3500 2050 50  0001 C CNN
F 1 "+5V" H 3515 2373 50  0000 C CNN
F 2 "" H 3500 2200 50  0001 C CNN
F 3 "" H 3500 2200 50  0001 C CNN
	1    3500 2200
	1    0    0    -1  
$EndComp
Wire Wire Line
	4950 2800 4550 2800
$Comp
L ece445_power:GND #PWR0102
U 1 1 5E757733
P 4150 2800
F 0 "#PWR0102" H 4150 2550 50  0001 C CNN
F 1 "GND" H 4155 2627 50  0000 C CNN
F 2 "" H 4150 2800 50  0001 C CNN
F 3 "" H 4150 2800 50  0001 C CNN
	1    4150 2800
	1    0    0    -1  
$EndComp
Connection ~ 3500 2200
Wire Wire Line
	3500 2200 3850 2200
Wire Wire Line
	4450 2200 4550 2200
$Comp
L ece445_device:C C1
U 1 1 5E7680AB
P 3500 2450
F 0 "C1" H 3615 2496 50  0000 L CNN
F 1 "C" H 3615 2405 50  0000 L CNN
F 2 "ece445_Capacitor_THT:CP_Radial_D5.0mm_P2.50mm" H 3538 2300 50  0001 C CNN
F 3 "~" H 3500 2450 50  0001 C CNN
	1    3500 2450
	1    0    0    -1  
$EndComp
$Comp
L ece445_device:C C3
U 1 1 5E768582
P 4550 2450
F 0 "C3" H 4665 2496 50  0000 L CNN
F 1 "2.2 uF" H 4665 2405 50  0000 L CNN
F 2 "ece445_Capacitor_THT:CP_Radial_D5.0mm_P2.50mm" H 4588 2300 50  0001 C CNN
F 3 "~" H 4550 2450 50  0001 C CNN
	1    4550 2450
	1    0    0    -1  
$EndComp
$Comp
L ece445_device:C C2
U 1 1 5E768B0C
P 3850 2650
F 0 "C2" H 3965 2696 50  0000 L CNN
F 1 "C" H 3965 2605 50  0000 L CNN
F 2 "ece445_Capacitor_THT:CP_Radial_D5.0mm_P2.50mm" H 3888 2500 50  0001 C CNN
F 3 "~" H 3850 2650 50  0001 C CNN
	1    3850 2650
	1    0    0    -1  
$EndComp
Wire Wire Line
	3500 2200 3500 2300
Wire Wire Line
	3500 2600 3500 2800
Connection ~ 3500 2800
Wire Wire Line
	3500 2800 3850 2800
Connection ~ 3850 2800
Wire Wire Line
	3850 2800 4150 2800
Wire Wire Line
	3850 2450 3850 2500
Wire Wire Line
	4550 2200 4550 2300
Connection ~ 4550 2200
Wire Wire Line
	4550 2200 4650 2200
Wire Wire Line
	4550 2600 4550 2800
Connection ~ 4550 2800
Wire Wire Line
	4550 2800 4150 2800
$EndSCHEMATC
