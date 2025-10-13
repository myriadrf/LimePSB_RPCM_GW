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

Building the Gateware
---------------------

To build the gateware:

- Clone the repository: :code:`git clone https://github.com/myriadrf/LimePSB_RPCM_GW.git`
- Generate the bitstream: :code:`python3 src/limepsb_rpcm.py --build`
- The bitstream will be generated at :code:`build/limepsb_rpcm_platform/gateware/limepsb_rpcm_platform.bin`.

Using from Host (Raspberry Pi)
------------------------------

The project includes a Python test script (:code:`test/test_gpsdo.py`) for controlling and
monitoring the GPSDO via SPI from a Raspberry Pi (e.g., CM4/CM5 on the LimePSB-RPCM board). This
script handles register configuration, enabling/disabling, error monitoring, and diagnostics.

Test Script Usage
^^^^^^^^^^^^^^^^^

The script supports:

- Configuring targets/tolerances (using ppm; note 1 ppm = 1000 ppb).
- Enabling/disabling/resetting the GPSDO.
- Dumping registers.
- Real-time monitoring of errors, DAC value, state, accuracy, and PPS activity.

Run with :code:`python3 test/test_gpsdo.py --help` for full options.

Example Commands
~~~~~~~~~~~~~~~~

Enable GPSDO with default 30.72 MHz clock and 0.1 ppm (100 ppb) tolerance::

    python3 test/test_gpsdo.py --enable

Enable with 10 MHz clock and 0.02 ppm (20 ppb) tolerance::

    python3 test/test_gpsdo.py --enable --clk-freq 10 --ppm 0.02

Monitor regulation loop indefinitely (press Ctrl+C to stop)::

    python3 test/test_gpsdo.py --check

Dump all registers once::

    python3 test/test_gpsdo.py --dump

Reset GPSDO (disable then re-enable after 2s delay)::

    python3 test/test_gpsdo.py --reset

Disable GPSDO::

    python3 test/test_gpsdo.py --disable

During monitoring, expect LED7 to blink red on active 1PPS. After ~few minutes, the state should
transition to "Fine Tune" with "3s Tune (Highest)" accuracy (100s interval). Errors should converge
near zero.

For custom integration, the script can serve as a reference for SPI driver implementation.

Documentation
-------------

More details can be found in:

-  /doc/ 