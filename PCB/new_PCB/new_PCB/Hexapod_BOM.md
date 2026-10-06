# Hexapod carrier board: BOM and TME order list

Generated from `PCB/new_PCB/new_PCB/new_PCB.kicad_sch` (schematic saved 2026-09-12). Order quantities are for **1 board**, with a few spares on the small parts.

> **About the links:** every TME link points to a real product page. TME rate-limited the automated checks, so **current stock was not verified**. Check stock and price when you add each item to the basket. If a part is out of stock, the *Specs* column says what an equivalent must match.

---

## 1. Order from TME

| # | Ref | Part | Specs | TME code | Need | Order | Link |
|---|-----|------|-------|----------|-----:|------:|------|
| 1 | Q1, Q2 | Vishay **SUD50P04-08-GE3** | P-MOSFET, −40 V, −50 A, 8 mΩ, DPAK/TO-252 (1 = G, 2 = D/tab, 3 = S) | SUD50P04-08-GE3 | 2 | 3 | [TME](https://www.tme.eu/pl/details/sud50p04-08-ge3/) |
| 2 | U3 | Nexperia **74LVC125AD,118** | quad tri-state buffer, SO14 (SOT108-1) | 74LVC125AD.118 | 1 | 2 | [TME](https://www.tme.eu/pl/details/74lvc125ad.118/) |
| 3 | U4 | Nexperia **74LVC14AD,118** | hex Schmitt inverter, SO14 (SOT108-1) | 74LVC14AD.118 | 1 | 2 | [TME](https://www.tme.eu/pl/details/74lvc14ad.118/) |
| 4 | D1, D2 | Nexperia **BZV55-B15,115** | Zener 15 V ±2 %, 0.5 W, SOD80C (MiniMELF) | BZV55-B15.115 | 2 | 5 | [TME](https://www.tme.eu/pl/details/bzv55-b15.115/) |
| 5 | J3–J8 | Molex **22-03-5035** (Mini-SPOX) | 3-pin header, 2.5 mm pitch, straight, THT, 3 A. The header ROBOTIS specifies for the AX-12A | MX-5267-03A | 6 | 7 | [TME](https://www.tme.eu/pl/details/mx-5267-03a/) |
| 6 | C7, C8, C9, C12, C15, C17, C21 | Samxon **EKM227M1EE11RRSHP** | 220 µF / **25 V**, Ø6.3 × 11 mm, **2.5 mm pitch** | KM220/25 | 7 | 10 | [TME](https://www.tme.eu/pl/details/km220_25/) |
| 7 | C6 | Samwha **SD2A226M6L011PC** | 22 µF / 100 V, Ø6.3 × 11 mm, 2.5 mm pitch (see note D) | SD2A226M6L011PC | 1 | 2 | [TME](https://www.tme.eu/pl/details/sd2a226m6l011pc/) |
| 8 | C19, C20 | Samsung **CL21A106KAYNNNE** | 10 µF / 25 V, X5R, 0805 | CL21A106KAYNNNE | 2 | 5 | [TME](https://www.tme.eu/pl/details/cl21a106kaynnne/) |
| 9 | C1, C3, C4, C5, C10, C11, C13, C14, C16, C18, C22 | Walsin **0805B104K500CT** | 100 nF / 50 V, X7R, 0805 (the same part you bought before) | 0805B104K500CT | 11 | 20 | [TME](https://www.tme.eu/pl/details/0805b104k500ct/) |
| 10 | R14, R15 | Royalohm **0805S8J0220T5E** | 22 Ω, 5 %, 0805 (sold in packs of 100) | SMD0805-22R | 2 | 100 | [TME](https://www.tme.eu/pl/details/smd0805-22r/) |
| 11 | U1 socket | Connfly **DS1023-1\*40S21** | female socket 1 × 40, 2.54 mm, straight, THT. **Cut each strip to 22** (see note B) | ZL262-40SG | 2 × 22 | 2 | [TME](https://www.tme.eu/pl/details/zl262-40sg/) |
| 12 | J9 | Connfly **DS1021-1\*2SF11-B** | male pin header 1 × 2, 2.54 mm, straight, THT | ZL201-02G | 1 | 2 | [TME](https://www.tme.eu/pl/details/zl201-02g/) |
| 13 | J9 | Ninigi **JUMPER-H/R** | 2.54 mm jumper, lets you run without the switch on the bench | JUMPER-H/R | 1 | 2 | [TME](https://www.tme.eu/pl/details/jumper-h_r/) |

## 2. Off-board protection and switch (TME)

| # | Part | Specs | TME code | Order | Link |
|---|------|-------|----------|------:|------|
| 14 | MTA **UNIVAL 20A** blade fuse | ATO/ATC 19 mm, 20 A, 32 V DC | AMF-20A | 3 | [TME](https://www.tme.eu/pl/details/amf-20a/) |
| 15 | Littelfuse **0FHA0002ZXJ** inline fuse holder | ATO/ATC 19 mm, **30 A** holder, 12 AWG leads (~10 cm), 32 V | 0FHA0002ZXJ | 1 | [TME](https://www.tme.eu/pl/details/0fha0002zxj/) |
| 16 | Ninigi **KN3(C)-101A-A1** toggle switch | SPST ON-OFF, 10 A / 250 V AC, **M3 screw terminals**, panel hole **Ø12.2 mm**. In stock (3930 pcs, checked 2026-09-24). Replaces KS132 (TS-20/BL), which is out of stock | TSP101AA1 | 1 | [TME](https://www.tme.eu/pl/details/tsp101aa1/) |

- **Fuse holder:** put it in the battery **+** lead **before** the Y-splitter, so one fuse protects both VBUS branches.
- **Switch:** it only interrupts the ESP32's 5 V (J9 carries about 0.5 A), so this switch has plenty of margin. Connect it to J9 with a 2-wire lead ending in a female 2.54 mm (DuPont) connector. A female–female jumper cable cut in half works.

## 3. Already have (don't order)

| Ref | Part | Qty needed | You have |
|-----|------|-----------:|---------:|
| J1, J2 | AMASS **XT60PW-M** (see note A) | 2 | 3 |
| C2 | Samwha RD1V107M6L011BB, 100 µF / 35 V, Ø6.3 × 11 | 1 | 20 |
| R1–R6, R10–R13 | Royalohm 10 kΩ 1 % 0805 | 10 | 100 |
| R7 | Royalohm 100 kΩ 1 % 0805 | 1 | 100 |
| R8 | Royalohm 22 kΩ 0805 | 1 | 100 |
| R9 | Royalohm 1 kΩ 1 % 0805 | 1 | 100 |
| C (100 nF) | Walsin 0805B104K500CT | (11 total, see item 9) | 3 |
| U2 | MP1584EN module | 1 | 1 |
| U1 | ESP32-S3-DevKitC-1 N16R8V | 1 | 1 |
| — | ToolkitRC XT60 Y-splitter (1 male → 2 female) | 1 | 1 |
| — | Battery, external cables | — | yes |
| — | AOD407 P-MOSFET ×3 | **don't use** (note C) | 3 |

## 4. Notes. Read before assembling.

**A. XT60 gender.** Your Y-splitter has **female** outputs, so the board needs **male** connectors. The **XT60PW-M** you already have is correct, even though the schematic value and footprint are named "XT60PW-F". Pin spacing matches (7.2 mm). Before soldering, hold one against the board and check that its two side retention legs line up with the oval slots.

**B. ESP32 sockets.** TME doesn't sell a 22-pin female socket, so cut a 40-pin strip. Cut **through the 23rd pin position** with a fine saw or side cutters and file it flat; you lose that one position, which is why each 40-pin strip gives one 22-pin socket. Plug the devkit into both sockets while soldering them so they come out straight and parallel. This assumes your devkit has male pins already soldered on. If it doesn't, also order a 1 × 40 male strip (ZL201-40G) and cut two 22-pin pieces.

**C. AOD407 isn't suitable here.** Its datasheet gives R<sub>DS(on)</sub> ≤ 115 mΩ at −10 V and 50 °C/W junction-to-air. At about 4 A per branch that's ~1.8 W, roughly a 90 °C temperature rise in a DPAK on this board. The SUD50P04-08 (8 mΩ) dissipates ~0.13 W at the same current. Keep the AOD407s for low-current jobs.

**D. C6 alternative.** C6 (22 µF on the buck output) can be one of your spare **100 µF / 35 V** Samwha caps instead. Same footprint, and the extra bulk on a 5 V rail is harmless. If you do that, skip item 7.

**E. MP1584EN.** Set the output to **5.0 V** with a multimeter **before** you fit the module.

**F. Servo connectors.** Molex 22-03-5035 is the header ROBOTIS lists for the AX-12A (mating cable housing 50-37-5033, crimp 08-70-1039). Pinout: 1 = GND, 2 = VDD, 3 = DATA, which matches J3–J8 on the board.

**G. 100 nF stock.** TME's page for 0805B104K500CT showed conflicting stock info. If it's unavailable, any **100 nF / 50 V / X7R / 0805** works.

---

## 5. Full board BOM (for assembly)

| Ref | Value | Footprint | Qty |
|-----|-------|-----------|----:|
| C1, C3, C4, C5, C10, C11, C13, C14, C16, C18, C22 | 100 nF | 0805 | 11 |
| C2 | 100 µF | CP_Radial D6.3 P2.50 | 1 |
| C6 | 22 µF | CP_Radial D6.3 P2.50 | 1 |
| C7, C8, C9, C12, C15, C17, C21 | 220 µF | CP_Radial D6.3 P2.50 | 7 |
| C19, C20 | 10 µF | 0805 | 2 |
| D1, D2 | BZV55B15 | MiniMELF | 2 |
| J1, J2 | XT60 (fit XT60PW-M) | AMASS-XT60PW-F | 2 |
| J3–J8 | Molex 22-03-5035 | CONN_22035035_MOL | 6 |
| J9 | 1 × 2 header | PinHeader_1x02_P2.54 | 1 |
| Q1, Q2 | SUD50P04-08 | TO-252-2 | 2 |
| R1–R6, R10–R13 | 10 kΩ | 0805 | 10 |
| R7 | 100 kΩ | 0805 | 1 |
| R8 | 22 kΩ | 0805 | 1 |
| R9 | 1 kΩ | 0805 | 1 |
| R14, R15 | 22 Ω | 0805 | 2 |
| U1 | ESP32-S3-DevKitC-1 N16R8V (on 2 × 22 sockets) | — | 1 |
| U2 | MP1584EN module | — | 1 |
| U3 | 74LVC125A | SO14 | 1 |
| U4 | 74LVC14A | SO14 | 1 |

**Polarity:** electrolytics have pad 1 = + (square pad). BZV55: the cathode band goes toward the VBUS side (pin 1 = K).



Numer pozycji
Symbol TME
Symbol Klienta
Cena brutto
Ilość zamówiona
Wartość pozycji brutto
% VAT
Numer oferty
Dostawa kompletna pozycji
Waga
Opcje
1

SUD50P04-08-GE3
9.594 PLN
3
szt
28.78 PLN
23 %

1.22 g

3

BZV55-B15.115
0.461 PLN
5
szt
2.31 PLN
23 %

0.21 g

4

KM220/25
0.381 PLN
20
szt
7.62 PLN
23 %

0.01 kg

5

SD2A226M6L011PC
0.398 PLN
20
szt
7.96 PLN
23 %

0.01 kg

6

CL21A106KAYNNNE
0.535 PLN
10
szt
5.35 PLN
23 %

0.3 g

7

0805B104K500CT
0.299 PLN
20
szt
5.98 PLN
23 %

0.6 g

8

SMD0805-22R
0.069 PLN
100
szt
6.95 PLN
23 %

2.4 g

9

ZL262-40SG
1.333 PLN
10
szt
13.33 PLN
23 %

0.03 kg

10

ZL201-02G
0.136 PLN
50
szt
6.79 PLN
23 %

5.35 g

11

JUMPER-H/R
0.423 PLN
20
szt
8.45 PLN
23 %

2.3 g

12

AMF-20A
0.496 PLN
10
szt
4.96 PLN
23 %

0.01 kg

14

R3-47A
8.45 PLN
1
szt
8.45 PLN
23 %

0.03 kg

15

TD1-1A-DC-3-R
30.848 PLN
1
szt
30.85 PLN
23 %

0.04 kg

16

UL-CSA-AWG12-R
9.791 PLN
1
m
9.79 PLN
23 %

0.05 kg

17

UL-CSA-AWG12-BK
10.48 PLN
1
m
10.48 PLN
23 %

0.04 kg

