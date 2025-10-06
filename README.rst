LimePSB-RPCM GPSDO gateware
===========================

This project provides FPGA gateware for the **GPS Disciplined Oscillator (GPSDO)** module on the
**LimePSB-RPCM** board, leveraging the **Lattice iCE40** FPGA. It receives a **1PPS (Pulse Per Second)**
signal from the GPS module or alternative sources, along with a clock from a
**Voltage-Controlled Temperature-Compensated Crystal Oscillator (VCTCXO)**. The design calculates
frequency errors over **1s**, **10s**, and **100s** averaging intervals to discipline the VCTCXO.

Host communication occurs via **SPI**, enabling module enable/disable, parameter configuration,
and frequency error retrieval. This gateware reuses the generic **LimePPSDO** core for core GPSDO
functionality (PPS detection, VCTCXO taming, and regulation), integrated with board-specific
elements like clock selection and SPI DAC control.

Key features include:

- **Multi-Source PPS Support**: GNSS TPULSE (default), RPI_SYNC_OUT, or RPI_SYNC_IN.
- **Clock Selection**: 10 MHz (LMK10_CLK_OUT0) or 30.72 MHz (LMKRF_CLK_OUT4) for VCTCXO.
- **SPI DAC Control**: Shared with RPI/CM4/CM5; overridden by GPSDO when enabled.
- **Regulation Algorithm**: Coarse and fine tuning via SERV RISC-V soft-core CPU for ±1 Hz (coarse)
  and ~0.01 Hz (fine) precision.
- **Error Monitoring**: 32-bit signed errors with configurable tolerances; interrupts on exceedance.
- **Status Indication**: LED7 blinks red on active 1PPS when enabled.

This design ensures high stability (<20 ppb tolerance configurable) for RF applications on the
LimePSB-RPCM board.

Available branches
------------------

-  **master** - most recent stable and tested work


Documentation
-------------

More details can be found in:

-  /doc/ 