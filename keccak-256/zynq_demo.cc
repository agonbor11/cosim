

#define SC_INCLUDE_DYNAMIC_PROCESSES

#include <inttypes.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include "systemc.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"
#include "tlm_utils/tlm_quantumkeeper.h"

using namespace sc_core;
using namespace sc_dt;
using namespace std;

#include "trace.h"
#include "iconnect.h"
#include "debugdev.h"
#include "soc/xilinx/zynq/xilinx-zynq.h"

#include "tlm2axilite-bridge.h"
#include "axi2tlm-bridge.h"
#include "demo-dma.h"

#include "Vmatmul.h"
#include "Vaxilite_dev.h"


#ifdef HAVE_VERILOG_VCS
#include "apb_slave_timer.h"
#endif

#ifdef HAVE_VERILOG_VERILATOR



#endif

#define AXIFULL_DATA_WIDTH 32
#define AXIFULL_ID_WIDTH 8
#define NR_DEMODMA  1    
#define NR_MASTERS	2 + NR_DEMODMA 
#define NR_DEVICES	3 + NR_DEMODMA

SC_MODULE(Top)
{
	iconnect<NR_MASTERS, NR_DEVICES> bus;
	xilinx_zynq zynq;
	debugdev debug;
	demodma *dma[NR_DEMODMA];
	sc_signal<bool> rst, rst_n;
	SC_HAS_PROCESS(Top);
sc_clock *clk;
    Vmatmul *al;
	tlm2axilite_bridge<6, 32> *tlm2axi_al;
    axi2tlm_bridge<64, AXIFULL_DATA_WIDTH,1,8,2,1,1,1,1,1> *axi2tlm_af;
//////creacion de señales para conectar con los bridges

//axilite
	sc_signal<bool> al_awvalid;
	sc_signal<bool> al_awready;
	sc_signal<sc_bv<6> > al_awaddr;
	sc_signal<sc_bv<3> > al_awprot;

	sc_signal<bool> al_wvalid;
	sc_signal<bool> al_wready;
	sc_signal<sc_bv<32> > al_wdata;
	sc_signal<sc_bv<4> > al_wstrb;

	sc_signal<bool> al_bvalid;
	sc_signal<bool> al_bready;
	sc_signal<sc_bv<2> > al_bresp;

	sc_signal<bool> al_arvalid;
	sc_signal<bool> al_arready;
	sc_signal<sc_bv<6> > al_araddr;
	sc_signal<sc_bv<3> > al_arprot;

	sc_signal<bool> al_rvalid;
	sc_signal<bool> al_rready;
	sc_signal<sc_bv<32> > al_rdata;
	sc_signal<sc_bv<2> > al_rresp;

//axifull
	sc_signal<bool> af_awvalid;
	sc_signal<bool> af_awready;
	sc_signal<sc_bv<64> > af_awaddr;
	sc_signal<sc_bv<3> > af_awprot;
    sc_signal<bool> af_awuser;
	sc_signal<sc_bv<4> > af_awregion;
	sc_signal<sc_bv<4> > af_awqos;
	sc_signal<sc_bv<4> > af_awcache;
	sc_signal<sc_bv<2> > af_awburst;
	sc_signal<sc_bv<3> > af_awsize;
	sc_signal<sc_bv<8> > af_awlen;
	sc_signal<bool > af_awid;
	sc_signal<sc_bv<2> > af_awlock;

    sc_signal<bool> af_wid;
    sc_signal<bool> af_wvalid;
	sc_signal<bool> af_wready;
	sc_signal<sc_bv<AXIFULL_DATA_WIDTH> > af_wdata;
	sc_signal<sc_bv<AXIFULL_DATA_WIDTH/8> > af_wstrb;
    sc_signal<bool> af_wuser;
	sc_signal<bool> af_wlast;

	sc_signal<bool> af_bvalid;
	sc_signal<bool> af_bready;
	sc_signal<sc_bv<2> > af_bresp;
	sc_signal<bool> af_buser;
    sc_signal<bool> af_bid;

	sc_signal<bool> af_arvalid;
	sc_signal<bool> af_arready;
	sc_signal<sc_bv<64> > af_araddr;
	sc_signal<sc_bv<3> > af_arprot;
   	sc_signal<bool> af_aruser;
	sc_signal<sc_bv<4> > af_arregion;
	sc_signal<sc_bv<4> > af_arqos;
	sc_signal<sc_bv<4> > af_arcache;
	sc_signal<sc_bv<2> > af_arburst;
	sc_signal<sc_bv<3> > af_arsize;
	sc_signal<sc_bv<8> > af_arlen;
    sc_signal<bool > af_arid;
	sc_signal<sc_bv<2> > af_arlock;

	sc_signal<bool> af_rvalid;
	sc_signal<bool> af_rready;
	sc_signal<sc_bv<AXIFULL_DATA_WIDTH> > af_rdata;
	sc_signal<sc_bv<2> > af_rresp;
    sc_signal<bool> af_ruser;
    sc_signal<bool> af_rid;
	sc_signal<bool> af_rlast;

	void pull_reset(void) {
		/* Pull the reset signal.  */
		rst.write(true);
		wait(1, SC_US);
		rst.write(false);
	}

void gen_rst_n(void)
	{
		rst_n.write(!rst.read());
	}

	Top(sc_module_name name, const char *sk_descr, sc_time quantum) :
		bus("bus"),
		zynq("zynq", sk_descr),
		debug("debug"),
		rst("rst"),
		rst_n("rst_n"),
//Señales
/////axilite
		al_awvalid("al_awvalid"),
		al_awready("al_awready"),
		al_awaddr("al_awaddr"),
		al_awprot("al_awprot"),

		al_wvalid("al_wvalid"),
		al_wready("al_wready"),
		al_wdata("al_wdata"),
		al_wstrb("al_wstrb"),

		al_bvalid("al_bvalid"),
		al_bready("al_bready"),
		al_bresp("al_bresp"),

		al_arvalid("al_arvalid"),
		al_arready("al_arready"),
		al_araddr("al_araddr"),
		al_arprot("al_arprot"),

		al_rvalid("al_rvalid"),
		al_rready("al_rready"),
		al_rdata("al_rdata"),
		al_rresp("al_rresp"),

/////axifull
		af_awvalid("af_awvalid"),
		af_awready("af_awready"),
		af_awaddr("af_awaddr"),
		af_awprot("af_awprot"),
		af_awuser("af_awuser"),
		af_awregion("af_awregion"),
		af_awqos("af_awqos"),
		af_awcache("af_awcache"),
		af_awburst("af_awburst"),
		af_awsize("af_awsize"),
		af_awlen("af_awlen"),
		af_awid("af_awid"),
		af_awlock("af_awlock"),

		af_wvalid("af_wvalid"),
		af_wready("af_wready"),
		af_wdata("af_wdata"),
		af_wstrb("af_wstrb"),
		af_wuser("af_wuser"),
		af_wlast("af_wlast"),

		af_bvalid("af_bvalid"),
		af_bready("af_bready"),
		af_bresp("af_bresp"),
		af_buser("af_buser"),
		af_bid("af_bid"),

		af_arvalid("af_arvalid"),
		af_arready("af_arready"),
		af_araddr("af_araddr"),
		af_arprot("af_arprot"),
		af_aruser("af_aruser"),
		af_arregion("af_arregion"),
		af_arqos("af_arqos"),
		af_arcache("af_arcache"),
		af_arburst("af_arburst"),
		af_arsize("af_arsize"),
		af_arlen("af_arlen"),
		af_arid("af_arid"),
		af_arlock("af_arlock"),

		af_rvalid("af_rvalid"),
		af_rready("af_rready"),
		af_rdata("af_rdata"),
		af_rresp("af_rresp"),
		af_ruser("af_ruser"),
		af_rid("af_rid"),
		af_rlast("af_rlast")
	{
        unsigned int i;
        SC_METHOD(gen_rst_n);
		sensitive << rst;
		m_qk.set_global_quantum(quantum);
		zynq.rst(rst);


	bus.memmap(0x40000000ULL, 0x100 - 1,
				ADDRMODE_RELATIVE, -1, debug.socket);

	for (i = 0; i < (sizeof dma / sizeof dma[0]); i++) {
			char name[16];
			snprintf(name, sizeof name, "demodma%d", i);
			dma[i] = new demodma(name);
            cout<<" dma[i]: "<<dma[i]<<endl;
		}
	for (i = 0; i < (sizeof dma / sizeof dma[0]); i++) {
               cout<<" dma[i]:"<<dma[i]<<endl;
			bus.memmap(0x50000000ULL + 0x100 * i, 0x18 - 1,
				ADDRMODE_RELATIVE, -1, dma[i]->tgt_socket);
		}
		tlm2axi_al = new tlm2axilite_bridge<6, 32> ("tlm2axi-al-bridge"); 
		axi2tlm_af = new axi2tlm_bridge<64, AXIFULL_DATA_WIDTH,1,8,2,1,1,1,1,1> ("axi2tlm-af-bridge");
		bus.memmap(0x60000000ULL, 0x100 - 1,
				ADDRMODE_RELATIVE, -1, tlm2axi_al->tgt_socket); 
		axi2tlm_af->socket.bind(*(bus.t_sk[2]));
		bus.memmap(0x00000000ULL,0x40000000 - 1,ADDRMODE_RELATIVE, -1, *(zynq.s_axi_hp[0]));
	 
		/* Connect the PL irqs to the irq_pl_to_ps wires.  */
		debug.irq(zynq.pl2ps_irq[0]);

		for (i = 0; i < (sizeof dma / sizeof dma[0]); i++) {
			dma[i]->init_socket.bind(*(bus.t_sk[1 + i])); 
			dma[i]->irq(zynq.pl2ps_irq[1 + i]);
		}
   zynq.m_axi_gp[0]->bind(*(bus.t_sk[0])); 

clk = new sc_clock("clk", sc_time(10, SC_US));
al = new Vmatmul("matmul");

//////////////////conectar bridges a señales
//axilite bridge con señales
		tlm2axi_al->clk(*clk);
        tlm2axi_al->resetn(rst_n);

		tlm2axi_al->awvalid(al_awvalid);
		tlm2axi_al->awready(al_awready);
		tlm2axi_al->awaddr(al_awaddr);
		tlm2axi_al->awprot(al_awprot);

		tlm2axi_al->wvalid(al_wvalid);
		tlm2axi_al->wready(al_wready);
		tlm2axi_al->wdata(al_wdata);
		tlm2axi_al->wstrb(al_wstrb);

		tlm2axi_al->bvalid(al_bvalid);
		tlm2axi_al->bready(al_bready);
		tlm2axi_al->bresp(al_bresp);

		tlm2axi_al->arvalid(al_arvalid);
		tlm2axi_al->arready(al_arready);
		tlm2axi_al->araddr(al_araddr);
		tlm2axi_al->arprot(al_arprot);

		tlm2axi_al->rvalid(al_rvalid);
		tlm2axi_al->rready(al_rready);
		tlm2axi_al->rdata(al_rdata);
		tlm2axi_al->rresp(al_rresp);


        /* AXI FULL. con señales */
		axi2tlm_af->clk(*clk);
		axi2tlm_af->resetn(rst_n);

        axi2tlm_af->awvalid(af_awvalid);
		axi2tlm_af->awready(af_awready);
		axi2tlm_af->awaddr(af_awaddr);
		axi2tlm_af->awprot(af_awprot);
		axi2tlm_af->awuser(af_awuser);
		axi2tlm_af->awregion(af_awregion);
		axi2tlm_af->awqos(af_awqos);
		axi2tlm_af->awcache(af_awcache);
		axi2tlm_af->awburst(af_awburst);
		axi2tlm_af->awsize(af_awsize);
		axi2tlm_af->awlen(af_awlen);
		axi2tlm_af->awid(af_awid);
		axi2tlm_af->awlock(af_awlock);

		axi2tlm_af->wvalid(af_wvalid);
		axi2tlm_af->wready(af_wready);
		axi2tlm_af->wdata(af_wdata);
		axi2tlm_af->wstrb(af_wstrb);
		axi2tlm_af->wuser(af_wuser);
		axi2tlm_af->wlast(af_wlast);

		axi2tlm_af->bvalid(af_bvalid);
		axi2tlm_af->bready(af_bready);
		axi2tlm_af->bresp(af_bresp);
		axi2tlm_af->buser(af_buser);
		axi2tlm_af->bid(af_bid);

		axi2tlm_af->arvalid(af_arvalid);
		axi2tlm_af->arready(af_arready);
		axi2tlm_af->araddr(af_araddr);
		axi2tlm_af->arprot(af_arprot);
		axi2tlm_af->aruser(af_aruser);
		axi2tlm_af->arregion(af_arregion);
		axi2tlm_af->arqos(af_arqos);
		axi2tlm_af->arcache(af_arcache);
		axi2tlm_af->arburst(af_arburst);
		axi2tlm_af->arsize(af_arsize);
		axi2tlm_af->arlen(af_arlen);
		axi2tlm_af->arid(af_arid);
		axi2tlm_af->arlock(af_arlock);

		axi2tlm_af->rvalid(af_rvalid);
		axi2tlm_af->rready(af_rready);
		axi2tlm_af->rdata(af_rdata);
		axi2tlm_af->rresp(af_rresp);
		axi2tlm_af->ruser(af_ruser);
		axi2tlm_af->rid(af_rid);
		axi2tlm_af->rlast(af_rlast);


/////IP con señales, tanto del tlm2axilite como del axi2tlm
    al->ap_clk(*clk);
    al->ap_rst_n(rst_n);
	// señales de tlm2axilite con IP
   al->s_axi_control_AWVALID(al_awvalid);
    al->s_axi_control_AWREADY(al_awready);
    al->s_axi_control_AWADDR(al_awaddr);

    al->s_axi_control_WVALID(al_wvalid);
    al->s_axi_control_WREADY(al_wready);
    al->s_axi_control_WDATA(al_wdata);
    al->s_axi_control_WSTRB(al_wstrb);

    al->s_axi_control_BVALID(al_bvalid);
    al->s_axi_control_BREADY(al_bready);
    al->s_axi_control_BRESP(al_bresp);

    al->s_axi_control_ARVALID(al_arvalid);
    al->s_axi_control_ARREADY(al_arready);
    al->s_axi_control_ARADDR(al_araddr);


    al->s_axi_control_RVALID(al_rvalid);
    al->s_axi_control_RREADY(al_rready);
    al->s_axi_control_RDATA(al_rdata);
    al->s_axi_control_RRESP(al_rresp);

///////// señales de axi2tlm y IP
    al->m_axi_data_AWVALID(af_awvalid);
    al->m_axi_data_AWREADY(af_awready);
    al->m_axi_data_AWADDR(af_awaddr);
    al->m_axi_data_AWPROT(af_awprot);
    al->m_axi_data_AWUSER(af_awuser);
    al->m_axi_data_AWREGION(af_awregion);
    al->m_axi_data_AWQOS(af_awqos);
    al->m_axi_data_AWCACHE(af_awcache);
    al->m_axi_data_AWBURST(af_awburst);
    al->m_axi_data_AWSIZE(af_awsize);
    al->m_axi_data_AWLEN(af_awlen);
    al->m_axi_data_AWID(af_awid);
    al->m_axi_data_AWLOCK(af_awlock);
    al->m_axi_data_WID(af_wid);
    al->m_axi_data_WVALID(af_wvalid);
    al->m_axi_data_WREADY(af_wready);
    al->m_axi_data_WDATA(af_wdata);
    al->m_axi_data_WSTRB(af_wstrb);
    al->m_axi_data_WUSER(af_wuser);
    al->m_axi_data_WLAST(af_wlast);
    al->m_axi_data_BVALID(af_bvalid);
    al->m_axi_data_BREADY(af_bready);
    al->m_axi_data_BRESP(af_bresp);
    al->m_axi_data_BUSER(af_buser);
    al->m_axi_data_BID(af_bid);
    al->m_axi_data_ARVALID(af_arvalid);
    al->m_axi_data_ARREADY(af_arready);
    al->m_axi_data_ARADDR(af_araddr);
    al->m_axi_data_ARPROT(af_arprot);
    al->m_axi_data_ARUSER(af_aruser);
    al->m_axi_data_ARREGION(af_arregion);
    al->m_axi_data_ARQOS(af_arqos);
    al->m_axi_data_ARCACHE(af_arcache);
    al->m_axi_data_ARBURST(af_arburst);
    al->m_axi_data_ARSIZE(af_arsize);
    al->m_axi_data_ARLEN(af_arlen);
    al->m_axi_data_ARID(af_arid);
    al->m_axi_data_ARLOCK(af_arlock);

    al->m_axi_data_RVALID(af_rvalid);
    al->m_axi_data_RREADY(af_rready);
    al->m_axi_data_RDATA(af_rdata);
    al->m_axi_data_RRESP(af_rresp);
    al->m_axi_data_RUSER(af_ruser);
    al->m_axi_data_RID(af_rid);
    al->m_axi_data_RLAST(af_rlast);


    al->__SYM__interrupt(zynq.pl2ps_irq[2]);

		/* Tie off any remaining unconnected signals.  */
		zynq.tie_off();

		SC_THREAD(pull_reset);
	}

private:
	tlm_utils::tlm_quantumkeeper m_qk;
};

void usage(void)
{
	cout << "tlm socket-path sync-quantum-ns" << endl;
}

int sc_main(int argc, char* argv[])
{
	Top *top;
	uint64_t sync_quantum;
	sc_trace_file *trace_fp = NULL;

	if (argc < 3) {
		sync_quantum = 10000;
	} else {
		sync_quantum = strtoull(argv[2], NULL, 10);
	}

	sc_set_time_resolution(1, SC_PS);

	top = new Top("top", argv[1], sc_time((double) sync_quantum, SC_NS));

	if (argc < 3) {
		sc_start(1, SC_PS);
		sc_stop();
		usage();
		exit(EXIT_FAILURE);
	}

	trace_fp = sc_create_vcd_trace_file("trace");
	trace(trace_fp, *top, top->name());

	sc_start();
	if (trace_fp) {
		sc_close_vcd_trace_file(trace_fp);
	}
	return 0;
}
