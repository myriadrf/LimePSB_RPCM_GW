#include <stdbool.h>
#include <stdint.h>
#include "vctcxo_tamer.h"
#include <neo430.h>
//#include "xparameters.h"
//#include "xil_io.h"


/* Define a cached version of the VCTCXO tamer control register */
uint8_t vctcxo_tamer_ctrl_reg = 0x00;

/* Define a global variable containing the current VCTCXO DAC setting.
 * This is a 'cached' value of what is written to the DAC and is used
 * by the VCTCXO calibration algorithm to avoid constant read requests
 * going out to the DAC. Initial power-up state of the DAC is mid-scale.
 */
uint16_t vctcxo_trim_dac_value = 0x77FA;

uint8_t vctcxo_tamer_read(uint8_t addr) {
    //TODO: replace with neo430 wishbone functions
    //return (uint8_t)Xil_In8(XPAR_AVMM_M0_BASEADDR + addr);
    neo430_wishbone32_read_start((uint32_t) addr);

    uint16_t timeout = 0;

    while(1){
      if (!neo430_wishbone_busy())
        break;
      if (timeout++ == 100) {
        #ifdef DEBUG_PRINT
            neo430_uart_br_print("\nError! Device not responding! Press key to proceed.\n");
        #endif
        neo430_wishbone_terminate(); // terminate current transfer
        #ifdef DEBUG_PRINT
            while(!neo430_uart_char_received());
        #endif
        return 0;
      }
    }

    return (uint8_t)neo430_wishbone32_get_data();

}

void vctcxo_tamer_write(uint8_t addr, uint8_t data) {
    //TODO: replace with neo430 wishbone functions
    //Xil_Out8(XPAR_AVMM_M0_BASEADDR+addr, data);

    neo430_wishbone32_write_start((uint32_t)addr, (uint32_t)data);

      // wait for transfer to finish
    uint16_t timeout = 0;
    while(1){
        if (!neo430_wishbone_busy())
         break;
        if (timeout++ == 100) {
            #ifdef DEBUG_PRINT
                neo430_uart_br_print("\nError! Device not responding! Press key to proceed.\n");
            #endif
            neo430_wishbone_terminate(); // terminate current transfer
            #ifdef DEBUG_PRINT
                while(!neo430_uart_char_received());
            #endif
            return;
        }
    }



}

void vctcxo_tamer_reset_counters(bool reset) {
    if( reset ) {
        vctcxo_tamer_ctrl_reg |= VT_CTRL_RESET;
    } else {
        vctcxo_tamer_ctrl_reg &= ~VT_CTRL_RESET;
    }

    vctcxo_tamer_write(VT_CTRL_ADDR, vctcxo_tamer_ctrl_reg);
    return;
}

void vctcxo_tamer_enable_isr(bool enable) {
    if( enable ) {
        vctcxo_tamer_ctrl_reg |= VT_CTRL_IRQ_EN;
    } else {
        vctcxo_tamer_ctrl_reg &= ~VT_CTRL_IRQ_EN;
    }

    vctcxo_tamer_write(VT_CTRL_ADDR, vctcxo_tamer_ctrl_reg);
    return;
}

void vctcxo_tamer_clear_isr() {
    vctcxo_tamer_write(VT_CTRL_ADDR, vctcxo_tamer_ctrl_reg | VT_CTRL_IRQ_CLR);
    return;
}

void vctcxo_tamer_set_tune_mode(vctcxo_tamer_mode mode) {

    switch (mode) {
        case VCTCXO_TAMER_DISABLED:
        case VCTCXO_TAMER_1_PPS:
        case VCTCXO_TAMER_10_MHZ:
            vctcxo_tamer_enable_isr(false);
            break;

        default:
            /* Erroneous value */
            return;
    }

    /* Set tuning mode */
    vctcxo_tamer_ctrl_reg &= ~VT_CTRL_TUNE_MODE;
    vctcxo_tamer_ctrl_reg |= (((uint8_t) mode) << 6);
    vctcxo_tamer_write(VT_CTRL_ADDR, vctcxo_tamer_ctrl_reg);

    /* Reset the counters */
    vctcxo_tamer_reset_counters( true );

    /* Take counters out of reset if tuning mode is not DISABLED */
    if( mode != 0x00 ) {
        vctcxo_tamer_reset_counters( false );
    }

    switch (mode) {
        case VCTCXO_TAMER_1_PPS:
        case VCTCXO_TAMER_10_MHZ:
            vctcxo_tamer_enable_isr(true);
            break;

        default:
            /* Leave ISR disabled otherwise */
            break;
    }

    return;
}

int32_t vctcxo_tamer_read_count(uint8_t addr) {
    //uint32_t base = AVALON_MM_EXTERNAL_0_BASE;
    //TODO: ???
    //uint32_t base = XPAR_AVMM_M0_BASEADDR;
    uint8_t offset = addr;
    int32_t value = 0;

    //TODO: replace with neo430 wishbone functions
    //value  = Xil_In8(base + offset++);
    //value |= ((int32_t) Xil_In8(base + offset++)) << 8;
    //value |= ((int32_t) Xil_In8(base + offset++)) << 16;
    //value |= ((int32_t) Xil_In8(base + offset++)) << 24;

    value  = (int32_t)vctcxo_tamer_read((uint32_t) offset++);
    value |= (int32_t)(vctcxo_tamer_read((uint32_t) offset++)) << 8;
    value |= (int32_t)(vctcxo_tamer_read((uint32_t) offset++)) << 16;
    value |= (int32_t)(vctcxo_tamer_read((uint32_t) offset++)) << 24;
    
    return value;
}

void vctcxo_trim_dac_write(uint8_t cmd, uint16_t val)
{
	uint8_t tuned_val_lsb;
	uint8_t tuned_val_msb;

	tuned_val_lsb = (uint8_t) (val & 0x00FF);
	tuned_val_msb = (uint8_t) ((val & 0xFF00) >> 8);

    //write tuned val to VCTCXO_tamer MM registers
    vctcxo_tamer_write(VT_DAC_TUNNED_VAL_ADDR0, tuned_val_lsb);
    vctcxo_tamer_write(VT_DAC_TUNNED_VAL_ADDR1, tuned_val_msb);
    

    

}


void vctcxo_tamer_isr(void *context) {
    struct vctcxo_tamer_pkt_buf *pkt = (struct vctcxo_tamer_pkt_buf *)context;
    uint8_t error_status = 0x00;

    /* Disable interrupts */
    vctcxo_tamer_enable_isr( false );

    /* Reset (stop) the counters */
    vctcxo_tamer_reset_counters( true );

    /* Read the current count values */
    pkt->pps_1s_error   = vctcxo_tamer_read_count(VT_ERR_1S_ADDR);
    pkt->pps_10s_error  = vctcxo_tamer_read_count(VT_ERR_10S_ADDR);
    pkt->pps_100s_error = vctcxo_tamer_read_count(VT_ERR_100S_ADDR);

    /* Read the error status register */
    error_status = vctcxo_tamer_read(VT_STAT_ADDR);

    /* Set the appropriate flags in the packet buffer */
    pkt->pps_1s_error_flag   = (error_status & VT_STAT_ERR_1S)   ? true : false;
    pkt->pps_10s_error_flag  = (error_status & VT_STAT_ERR_10S)  ? true : false;
    pkt->pps_100s_error_flag = (error_status & VT_STAT_ERR_100S) ? true : false;

    /* Clear interrupt */
    vctcxo_tamer_clear_isr();

    /* Tell the main loop that there is a request pending */
    pkt->ready = true;

    return;
}


void vctcxo_tamer_init(){
    /* Default VCTCXO Tamer and its interrupts to be disabled. */
	vctcxo_tamer_write(VT_STATE_ADDR, 0x00);
	/* Write status to to state register*/
    vctcxo_tamer_set_tune_mode(VCTCXO_TAMER_1_PPS);
}

void vctcxo_tamer_dis(){
    /* Default VCTCXO Tamer and its interrupts to be disabled. */
    vctcxo_tamer_set_tune_mode(VCTCXO_TAMER_DISABLED);

	/* Write status to to state register*/
	vctcxo_tamer_write(VT_STATE_ADDR, 0x00);
}
