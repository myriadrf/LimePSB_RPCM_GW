create_clock -period 100.000000 -name lmk_clk10     [get_ports LMK10_CLK_OUT0]
create_clock -period  32.552083 -name lmk_clk30p72  [get_ports LMKRF_CLK_OUT4]

create_clock -period  1000000000 -name gnss_tpulse  [get_ports GNSS_TPULSE]
