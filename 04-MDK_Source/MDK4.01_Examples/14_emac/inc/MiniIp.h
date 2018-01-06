/* ----------------------------------------------------------------------------
 *         ATMEL Microcontroller Software Support 
 * ----------------------------------------------------------------------------
 * Copyright (c) 2008, Atmel Corporation
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaimer below.
 *
 * Atmel's name may not be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ----------------------------------------------------------------------------
 */

#ifndef _MINIIP_H
#define _MINIIP_H

//-----------------------------------------------------------------------------
//         Define
//-----------------------------------------------------------------------------

/// Ethernet types
#define ETH_PROT_IP             0x0800 // 2048  (0x0800) IPv4
#define ETH_PROT_ARP            0x0806 // 2054  (0x0806) ARP
                                       // 32923 (0x8019) Appletalk
                                       // 34525 (0x86DD) IPv6

/// ARP OP codes
#define ARP_REQUEST             0x0001
#define ARP_REPLY               0x0002

/// IP protocoles
// http://www.iana.org/assignments/protocol-numbers
#define IP_PROT_ICMP            1
#define IP_PROT_IP              4
#define IP_PROT_TCP             6
#define IP_PROT_UDP             17

/// ICMP types
// http://www.iana.org/assignments/icmp-parameters
#define ICMP_ECHO_REPLY         0x00 // Echo reply (used to ping)
                            // 1 and 2  Reserved
#define ICMP_DEST_UNREACHABLE   0x03 // Destination Unreachable
#define ICMP_SOURCE_QUENCH      0x04 // Source Quench
#define ICMP_REDIR_MESSAGE      0x05 // Redirect Message
#define ICMP_ALT_HOST_ADD       0x06 // Alternate Host Address
                            //  0x07    Reserved
#define ICMP_ECHO_REQUEST       0x08 // Echo Request
#define ICMP_ROUTER_ADV         0x09 // Router Advertisement
#define ICMP_ROUTER_SOL         0x0A // Router Solicitation
#define ICMP_TIME_EXC           0x0B // Time Exceeded
#define ICMP_PARAM_PB           0x0C // Parameter Problem: Bad IP header
#define ICMP_TIMESTAMP          0x0D // Timestamp
#define ICMP_TIMESTAMP_REP      0x0E // Timestamp Reply
#define ICMP_INFO_REQ           0x0F // Information Request
#define ICMP_INFO_REPLY         0x10 // Information Reply
#define ICMP_ADD_MASK_REQ       0x11 // Address Mask Request
#define ICMP_ADD_MASK_REP       0x12 // Address Mask Reply
                            //  0x13    Reserved for security
                            //  0X14 through 0x1D Reserved for robustness experiment
#define ICMP_TRACEROUTE         0x1E // Traceroute
#define ICMP_DAT_CONV_ERROR     0x1F // Datagram Conversion Error
#define ICMP_MOB_HOST_RED       0x20 // Mobile Host Redirect
#define ICMP_W_A_Y              0x21 // Where-Are-You (originally meant for IPv6)
#define ICMP_H_I_A              0x22 // Here-I-Am (originally meant for IPv6)
#define ICMP_MOB_REG_REQ        0x23 // Mobile Registration Request
#define ICMP_MOB_REG_REP        0x24 // Mobile Registration Reply
#define ICMP_DOM_NAME_REQ       0x25 // Domain Name Request
#define ICMP_DOM_NAME_REP       0x26 // Domain Name Reply
#define ICMP_SKIP_ALGO_PROT     0x27 // SKIP Algorithm Discovery Protocol, Simple Key-Management for Internet Protocol
#define ICMP_PHOTURIS           0x28 // Photuris, Security failures
#define ICMP_EXP_MOBIL          0x29 // ICMP for experimental mobility protocols such as Seamoby [RFC4065]
                             // 0x2A through 0xFF  Reserved

//-----------------------------------------------------------------------------
//         Macros
//-----------------------------------------------------------------------------

/// Swap 2 bytes of a word
#define SWAP16(x)   (((x & 0xff) << 8) | (x >> 8))

//-----------------------------------------------------------------------------
//         Types
//-----------------------------------------------------------------------------

#ifdef __ICCARM__          // IAR
#pragma pack(1)            // IAR
#define __attribute__(...) // IAR
#endif                     // IAR

/// Ethernet header structure
typedef struct _EthHdr
{
    unsigned char       et_dest[6]; /// Destination node
    unsigned char       et_src[6];  /// Source node
    unsigned short      et_protlen; /// Protocol or length
} __attribute__ ((packed)) EthHeader, *PEthHeader;  // GCC

/// ARP header structure
typedef struct _ArpHdr
{
    unsigned short      ar_hrd;     /// Format of hardware address
    unsigned short      ar_pro;     /// Format of protocol address
    unsigned char       ar_hln;     /// Length of hardware address
    unsigned char       ar_pln;     /// Length of protocol address
    unsigned short      ar_op;      /// Operation
    unsigned char       ar_sha[6];  /// Sender hardware address
    unsigned char       ar_spa[4];  /// Sender protocol address
    unsigned char       ar_tha[6];  /// Target hardware address
    unsigned char       ar_tpa[4];  /// Target protocol address
} __attribute__ ((packed)) ArpHeader, *PArpHeader;  // GCC

/// IP Header structure
typedef struct _IPheader {
    unsigned char       ip_hl_v;    /// header length and version
    unsigned char       ip_tos;     /// type of service
    unsigned short      ip_len;     /// total length
    unsigned short      ip_id;      /// identification
    unsigned short      ip_off;     /// fragment offset field
    unsigned char       ip_ttl;     /// time to live
    unsigned char       ip_p;       /// protocol
    unsigned short      ip_sum;     /// checksum
    unsigned char       ip_src[4];  /// Source IP address
    unsigned char       ip_dst[4];  /// Destination IP address
    unsigned short      udp_src;    /// UDP source port
    unsigned short      udp_dst;    /// UDP destination port
    unsigned short      udp_len;    /// Length of UDP packet
    unsigned short      udp_xsum;   /// Checksum
} __attribute__ ((packed)) IpHeader, *PIpHeader;    // GCC

/// ICMP echo header structure
typedef struct _IcmpEchoHdr {
    unsigned char       type;       /// type of message
    unsigned char       code;       /// type subcode
    unsigned short      cksum;      /// ones complement cksum of struct
    unsigned short      id;         /// identifier
    unsigned short      seq;        /// sequence number
} __attribute__ ((packed)) IcmpEchoHeader, *PIcmpEchoHeader;    // GCC

/// Ethernet packet structure
typedef struct _EthPacket
{
    EthHeader           EthHdr;
    ArpHeader           ArpHdr;
} __attribute__ ((packed)) EthPacket, *PEthPacket;  // GCC

#ifdef __ICCARM__          // IAR
#pragma pack()             // IAR
#endif                     // IAR

//-----------------------------------------------------------------------------
//         Global functions
//-----------------------------------------------------------------------------

extern unsigned short IcmpChksum(unsigned short * p, int len);


#endif //  #ifndef _MINIIP_H
