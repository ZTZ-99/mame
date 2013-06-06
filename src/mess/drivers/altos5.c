/***************************************************************************

    Altos 5-15

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"
#include "machine/z80dart.h"
#include "machine/z80dma.h"
#include "machine/serial.h"


class altos5_state : public driver_device
{
public:
	altos5_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_pio0(*this, "z80pio_0"),
		m_pio1(*this, "z80pio_1"),
		m_dart(*this, "z80dart"),
		m_sio (*this, "z80sio"),
		m_ctc (*this, "z80ctc")
	{ }

	DECLARE_READ8_MEMBER(memory_read_byte);
	DECLARE_WRITE8_MEMBER(memory_write_byte);
	DECLARE_READ8_MEMBER(io_read_byte);
	DECLARE_WRITE8_MEMBER(io_write_byte);
	DECLARE_READ8_MEMBER(port08_r);
	DECLARE_READ8_MEMBER(port09_r);
	DECLARE_WRITE8_MEMBER(port08_w);
	DECLARE_WRITE8_MEMBER(port09_w);
	DECLARE_WRITE8_MEMBER(port14_w);
	DECLARE_DRIVER_INIT(altos5);
	DECLARE_WRITE8_MEMBER(kbd_put);
	DECLARE_READ8_MEMBER(port2e_r);
	DECLARE_READ8_MEMBER(port2f_r);
	DECLARE_WRITE_LINE_MEMBER(ctc_z1_w);
	UINT8 m_port08;
	UINT8 m_port09;
	bool m_ipl;
	bool m_wrpt;
	void setup_banks();
	virtual void machine_reset();
	required_device<cpu_device> m_maincpu;
	required_device<z80pio_device> m_pio0;
	required_device<z80pio_device> m_pio1;
	required_device<z80dart_device> m_dart;
	required_device<z80sio0_device> m_sio;
	required_device<z80ctc_device> m_ctc;
};

static ADDRESS_MAP_START(altos5_mem, AS_PROGRAM, 8, altos5_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x0fff ) AM_READ_BANK("bankr0") AM_WRITE_BANK("bankw0")
	AM_RANGE( 0x1000, 0x1fff ) AM_READ_BANK("bankr1") AM_WRITE_BANK("bankw1")
	AM_RANGE( 0x2000, 0x2fff ) AM_READ_BANK("bankr2") AM_WRITE_BANK("bankw2")
	AM_RANGE( 0x3000, 0x3fff ) AM_READ_BANK("bankr3") AM_WRITE_BANK("bankw3")
	AM_RANGE( 0x4000, 0x4fff ) AM_READ_BANK("bankr4") AM_WRITE_BANK("bankw4")
	AM_RANGE( 0x5000, 0x5fff ) AM_READ_BANK("bankr5") AM_WRITE_BANK("bankw5")
	AM_RANGE( 0x6000, 0x6fff ) AM_READ_BANK("bankr6") AM_WRITE_BANK("bankw6")
	AM_RANGE( 0x7000, 0x7fff ) AM_READ_BANK("bankr7") AM_WRITE_BANK("bankw7")
	AM_RANGE( 0x8000, 0x8fff ) AM_READ_BANK("bankr8") AM_WRITE_BANK("bankw8")
	AM_RANGE( 0x9000, 0x9fff ) AM_READ_BANK("bankr9") AM_WRITE_BANK("bankw9")
	AM_RANGE( 0xa000, 0xafff ) AM_READ_BANK("bankra") AM_WRITE_BANK("bankwa")
	AM_RANGE( 0xb000, 0xbfff ) AM_READ_BANK("bankrb") AM_WRITE_BANK("bankwb")
	AM_RANGE( 0xc000, 0xcfff ) AM_READ_BANK("bankrc") AM_WRITE_BANK("bankwc")
	AM_RANGE( 0xd000, 0xdfff ) AM_READ_BANK("bankrd") AM_WRITE_BANK("bankwd")
	AM_RANGE( 0xe000, 0xefff ) AM_READ_BANK("bankre") AM_WRITE_BANK("bankwe")
	AM_RANGE( 0xf000, 0xffff ) AM_READ_BANK("bankrf") AM_WRITE_BANK("bankwf")
ADDRESS_MAP_END

static ADDRESS_MAP_START(altos5_io, AS_IO, 8, altos5_state)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE_LEGACY("z80dma", z80dma_r, z80dma_w)
	//AM_RANGE(0x04, 0x07) // FD1797 fdc
	AM_RANGE(0x08, 0x0b) AM_DEVREADWRITE("z80pio_0", z80pio_device, read, write)
	AM_RANGE(0x0c, 0x0f) AM_DEVREADWRITE("z80ctc", z80ctc_device, read, write)
	AM_RANGE(0x10, 0x13) AM_DEVREADWRITE("z80pio_1", z80pio_device, read, write)
	AM_RANGE(0x14, 0x17) AM_WRITE(port14_w)
	AM_RANGE(0x1c, 0x1f) AM_DEVREADWRITE("z80dart", z80dart_device, ba_cd_r, ba_cd_w)
	//AM_RANGE(0x20, 0x23) // Hard drive
	AM_RANGE(0x2c, 0x2f) AM_DEVREADWRITE("z80sio", z80sio0_device, ba_cd_r, ba_cd_w)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( altos5 )
INPUT_PORTS_END

void altos5_state::setup_banks()
{
	UINT8 *prom =  memregion("proms")->base();
	UINT8 i;
	offs_t offset = ((~m_port09 & 0x20) << 3) | (m_port09 & 0xc0) | ((m_port09 & 0x18) << 1);
printf("\n%X:",offset);
	for (i = 0; i < 16; i++)
		printf("%X ",prom[offset+i]);
	membank("bankr0")->set_entry(49);
}

void altos5_state::machine_reset()
{
	m_port08 = 0;
	m_port09 = 0;
	m_wrpt = 0;
	m_ipl = 1;
	setup_banks();
}

static const z80_daisy_config daisy_chain_intf[] =
{
	{ "z80dma" },
	{ "z80pio_0" },
	{ "z80pio_1" },
	{ "z80ctc" },
	{ "z80dart" },
//	{ "z80sio" },
	{ NULL }
};

/*
d0: L = a HD is present
d1: L = a 2nd hard drive is present
d2: unused configuration input (must be H to skip HD boot)
d3: selected floppy is single(L) or double sided(H) 
d7: IRQ from FDC
*/
READ8_MEMBER( altos5_state::port08_r )
{
	return m_port08 | 0x87;
}

/*
d0: HD IRQ
*/
READ8_MEMBER( altos5_state::port09_r )
{
	return m_port09 | 0x01;
}

/*
d4: DDEN (H = double density
d5: DS (H = drive 2)
d6: SS (H = side 2)
*/
WRITE8_MEMBER( altos5_state::port08_w )
{
	m_port08 = data;
}

/*
d1, 2: Memory Map template selection (0 = diag; 1 = oasis; 2 = mp/m)
d3, 4: CPU bank select
d5:    H = Write protect of common area
d6, 7: DMA bank select
*/
WRITE8_MEMBER( altos5_state::port09_w )
{
	m_port09 = data;
	setup_banks();
}

// turns off IPL mode, removes boot rom from memory map
WRITE8_MEMBER( altos5_state::port14_w )
{
	m_ipl = 0;
	setup_banks();
}

READ8_MEMBER(altos5_state::memory_read_byte)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	return prog_space.read_byte(offset);
}

WRITE8_MEMBER(altos5_state::memory_write_byte)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	return prog_space.write_byte(offset, data);
}

READ8_MEMBER(altos5_state::io_read_byte)
{
	address_space& prog_space = m_maincpu->space(AS_IO);
	return prog_space.read_byte(offset);
}

WRITE8_MEMBER(altos5_state::io_write_byte)
{
	address_space& prog_space = m_maincpu->space(AS_IO);
	return prog_space.write_byte(offset, data);
}

static Z80DMA_INTERFACE( dma_intf )
{
	DEVCB_NULL, //DEVCB_DRIVER_LINE_MEMBER(altos5_state, p8k_dma_irq_w),
	DEVCB_CPU_INPUT_LINE("maincpu", INPUT_LINE_IRQ0),
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(altos5_state, memory_read_byte),
	DEVCB_DRIVER_MEMBER(altos5_state, memory_write_byte),
	DEVCB_DRIVER_MEMBER(altos5_state, io_read_byte),
	DEVCB_DRIVER_MEMBER(altos5_state, io_write_byte)
};

// baud rate generator and RTC. All inputs are 2MHz.
WRITE_LINE_MEMBER( altos5_state::ctc_z1_w )
{
	m_dart->rxca_w(state);
	m_dart->txca_w(state);
	m_sio->rxca_w(state);
	m_sio->txca_w(state);
}

static Z80CTC_INTERFACE( ctc_intf )
{
	DEVCB_CPU_INPUT_LINE("maincpu", INPUT_LINE_IRQ0), // interrupt callback
	DEVCB_DEVICE_LINE_MEMBER("z80sio", z80dart_device, rxtxcb_w),         /* ZC/TO0 callback - SIO Ch B */
	DEVCB_DRIVER_LINE_MEMBER(altos5_state, ctc_z1_w),         /* ZC/TO1 callback - Z80DART Ch A, SIO Ch A */
	DEVCB_DEVICE_LINE_MEMBER("z80dart", z80dart_device, rxtxcb_w),         /* ZC/TO2 callback - Z80DART Ch B */
};

// system functions
static Z80PIO_INTERFACE( pio0_intf )
{
	DEVCB_CPU_INPUT_LINE("maincpu", INPUT_LINE_IRQ0), // interrupt callback
	DEVCB_DRIVER_MEMBER(altos5_state, port08_r),         /* read port A */
	DEVCB_DRIVER_MEMBER(altos5_state, port08_w),         /* write port A */
	DEVCB_NULL,         /* portA ready active callback */
	DEVCB_DRIVER_MEMBER(altos5_state, port09_r),         /* read port B */
	DEVCB_DRIVER_MEMBER(altos5_state, port09_w),         /* write port B */
	DEVCB_NULL          /* portB ready active callback */
};

// parallel port
static Z80PIO_INTERFACE( pio1_intf )
{
	DEVCB_CPU_INPUT_LINE("maincpu", INPUT_LINE_IRQ0), // interrupt callback
	DEVCB_NULL,         /* read port A */
	DEVCB_NULL,         /* write port A */
	DEVCB_NULL,         /* portA ready active callback */
	DEVCB_NULL,         /* read port B */
	DEVCB_NULL,         /* write port B */
	DEVCB_NULL          /* portB ready active callback */
};

// serial printer and console#3
static Z80DART_INTERFACE( dart_intf )
{
	0, 0, 0, 0,

	// console#3
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,

	// printer
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,

	DEVCB_CPU_INPUT_LINE("maincpu", INPUT_LINE_IRQ0),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

static Z80SIO_INTERFACE( sio_intf )
{
	9600, 9600, 153600, 153600, // rxa, txa, rxb, txb clocks (from CTC)

	// console#2
	DEVCB_NULL, // ChA in data
	DEVCB_NULL, // out data
	DEVCB_NULL, // DTR
	DEVCB_NULL, // RTS
	DEVCB_NULL, // WRDY
	DEVCB_NULL, // SYNC

	// console#1
	DEVCB_DEVICE_LINE_MEMBER("rs232", serial_port_device, rx),
	DEVCB_DEVICE_LINE_MEMBER("rs232", serial_port_device, tx),
	DEVCB_DEVICE_LINE_MEMBER("rs232", rs232_port_device, dtr_w),
	DEVCB_DEVICE_LINE_MEMBER("rs232", rs232_port_device, rts_w),
	DEVCB_NULL,
	DEVCB_NULL,

	DEVCB_CPU_INPUT_LINE("maincpu", INPUT_LINE_IRQ0),
	DEVCB_NULL, // unused DRQ pins
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

static const rs232_port_interface rs232_intf =
{
	DEVCB_NULL,
	DEVCB_DEVICE_LINE_MEMBER("z80sio", z80dart_device, dcda_w),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DEVICE_LINE_MEMBER("z80sio", z80dart_device, ctsa_w)
};

DRIVER_INIT_MEMBER( altos5_state, altos5 )
{
	UINT8 *RAM = memregion("maincpu")->base();
	membank("bankr0")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankr1")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankr2")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankr3")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankr4")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankr5")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankr6")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankr7")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankr8")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankr9")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankra")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankrb")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankrc")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankrd")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankre")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankrf")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankw0")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankw1")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankw2")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankw3")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankw4")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankw5")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankw6")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankw7")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankw8")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankw9")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankwa")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankwb")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankwc")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankwd")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankwe")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankwf")->configure_entries(0, 50, &RAM[0], 0x1000);
}

static MACHINE_CONFIG_START( altos5, altos5_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_8MHz / 2)
	MCFG_CPU_PROGRAM_MAP(altos5_mem)
	MCFG_CPU_IO_MAP(altos5_io)
	MCFG_CPU_CONFIG(daisy_chain_intf)

	/* Devices */
	MCFG_Z80DMA_ADD( "z80dma",   XTAL_8MHz / 2, dma_intf)
	MCFG_Z80PIO_ADD( "z80pio_0", XTAL_8MHz / 2, pio0_intf )
	MCFG_Z80PIO_ADD( "z80pio_1", XTAL_8MHz / 2, pio1_intf )
	MCFG_Z80CTC_ADD( "z80ctc",   XTAL_8MHz / 2, ctc_intf )
	MCFG_Z80DART_ADD("z80dart",  XTAL_8MHz / 2, dart_intf )
	MCFG_Z80SIO0_ADD("z80sio",   XTAL_8MHz / 2, sio_intf )
	MCFG_RS232_PORT_ADD("rs232", rs232_intf, default_rs232_devices, "serial_terminal")
MACHINE_CONFIG_END


/* ROM definition */
ROM_START( altos5 ) // 00000-2FFFF = ram banks; 30000-30FFF wprt space; 31000-31FFF ROM
	ROM_REGION( 0x32000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD("2732.bin",   0x31000, 0x1000, CRC(15fdc7eb) SHA1(e15bdf5d5414ad56f8c4bb84edc6f967a5f01ba9)) // bios
	ROM_FILL(0x31054, 2, 0) // temp until banking sorted out
	ROM_FILL(0x31344, 3, 0) // kill self test

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD("82s141.bin", 0x0000, 0x0200, CRC(35c8078c) SHA1(dce24374bfcc5d23959e2c03485d82a119c0c3c9)) // banking control
ROM_END

/* Driver */

/*   YEAR  NAME    PARENT  COMPAT   MACHINE  INPUT   CLASS           INIT    COMPANY    FULLNAME       FLAGS */
COMP(1982, altos5, 0,      0,       altos5,  altos5, altos5_state,  altos5, "Altos", "Altos 5-15", GAME_NOT_WORKING | GAME_NO_SOUND)
