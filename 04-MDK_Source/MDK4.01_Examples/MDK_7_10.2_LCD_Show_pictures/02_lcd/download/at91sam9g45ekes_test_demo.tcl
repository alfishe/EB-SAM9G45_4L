################################################################################
#    SAM-BA history file
################################################################################

################################################################################
    global target
################################################################################

NANDFLASH::Init
GENERIC::EraseAll
GENERIC::SendBootFile "nandflash_at91sam9g45ekes.bin"
send_file {NandFlash} "lcd.bin" 0x20000 0

GENERIC::Init $RAM::appletAddr $RAM::appletMailboxAddr $RAM::appletFileName [list $::target(comType) $::target(traceLevel) $BOARD::extRamVdd $BOARD::extRamType $BOARD::extRamDataBusWidth $BOARD::extDDRamModel]
send_file {DDRAM} "image1_rgb.raw" 0x70300000 0
send_file {DDRAM} "image2_rgb.raw" 0x70700000 0


