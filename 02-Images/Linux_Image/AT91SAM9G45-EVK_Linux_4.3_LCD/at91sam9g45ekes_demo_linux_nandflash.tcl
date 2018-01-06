# ----------------------------------------------------------------------------
#         ATMEL Microcontroller Software Support 
# ----------------------------------------------------------------------------
# Copyright (c) 2008, Atmel Corporation
#
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# - Redistributions of source code must retain the above copyright notice,
# this list of conditions and the disclaimer below.
#
# Atmel's name may not be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# DISCLAIMER: THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
# DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
# OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
# LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
# EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# ----------------------------------------------------------------------------


################################################################################
#  proc uboot_env: Convert u-boot variables in a string ready to be flashed
#                  in the region reserved for environment variables
################################################################################
proc set_uboot_env {nameOfLstOfVar} {
    upvar $nameOfLstOfVar lstOfVar
    
    # sector size is the size defined in u-boot CFG_ENV_SIZE
    set sectorSize [expr 0x20000 - 5]

    set strEnv [join $lstOfVar "\0"]
    while {[string length $strEnv] < $sectorSize} {
        append strEnv "\0"
    }
    # \0 between crc and strEnv is the flag value for redundant environment
    set strCrc [binary format i [::vfs::crc $strEnv]]
    return "$strCrc\0$strEnv"
}

################################################################################
#  Main script: Load the linux demo in DataFlash,
#               Update the environment variables
################################################################################
set bootstrapFile	"nandflash_at91sam9g45ekes.bin"
set ubootFile		"u-boot.bin"	
set kernelFile		"uImage"
set rootfsFile		"Angstrom-x11-image-demo-glibc-at91.rootfs.jffs2"

set ubootEnvFile	"ubootEnvtFileNandFlash.bin"

## NandFlash Mapping
set bootStrapAddr	0x00000000
set ubootAddr		  0x00020000
set ubootEnvAddr  0x00060000
set kernelAddr		0x00200000
set rootfsAddr		0x00500000

# u-boot variable
set kernelUbootAddr	0x00200000 
set kernelLoadAddr	0x72200000


## NandFlash Mapping
set kernelSize    [format "0x%08X" [file size $kernelFile]]

lappend u_boot_variables \
    "ethaddr=3a:1f:34:08:54:54" \
    "bootdelay=3" \
    "baudrate=115200" \
    "stdin=serial" \
    "stdout=serial" \
    "stderr=serial" \
    "ipaddr=192.168.2.15" \
    "serverip=192.168.2.110" \
    "bootargs=mem=128M console=ttyS0,115200 mtdparts=atmel_nand:5M(bootstrap/uboot/kernel)ro,64M(rootfs),-(data) root=/dev/mtdblock1 rw   rootfstype=jffs2" \
    "bootcmd=nand read.i $kernelLoadAddr $kernelUbootAddr $kernelSize; bootm $kernelLoadAddr"

puts "-I- === Initialize the NAND access ==="
NANDFLASH::Init

puts "-I- === Erase all the NAND flash blocs and test the erasing ==="
NANDFLASH::EraseAllNandFlash

puts "-I- === Load the bootstrap: nandflash_at91sam9-ek in the first sector ==="
NANDFLASH::sendBootFile $bootstrapFile

puts "-I- === Load the u-boot in the next sectors ==="
send_file {NandFlash} "$ubootFile" $ubootAddr 0 

puts "-I- === Load the u-boot environment variables ==="
set fh [open "$ubootEnvFile" w]
fconfigure $fh -translation binary
puts -nonewline $fh [set_uboot_env u_boot_variables]
close $fh
send_file {NandFlash} "$ubootEnvFile" $ubootEnvAddr 0 

puts "-I- === Load the Kernel image ==="
send_file {NandFlash} "$kernelFile" $kernelAddr 0

puts "-I- === Load the linux file system ==="
send_file {NandFlash} "$rootfsFile" $rootfsAddr 0
