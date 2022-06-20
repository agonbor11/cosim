
enum {
	DEMODMA_CTRL_RUN  = 1 << 0,
	DEMODMA_CTRL_DONE = 1 << 1,
};

enum {
	DEMODMA_RESP_OKAY               = 0,
	DEMODMA_RESP_BUS_GENERIC_ERROR  = 1,
	DEMODMA_RESP_ADDR_DECODE_ERROR  = 2,
};

class demodma
: public sc_core::sc_module
{
public:
	tlm_utils::simple_initiator_socket<demodma> init_socket;
	tlm_utils::simple_target_socket<demodma> tgt_socket;

	sc_out<bool> irq;
	demodma(sc_core::sc_module_name name);
	SC_HAS_PROCESS(demodma);
private:
	union  { ///de este es de quien reserva luego espacio en la memoria
		struct {
			uint32_t dst_addr;
			uint32_t src_addr;
			uint32_t len;
			uint32_t ctrl;
			uint32_t byte_en;
			uint32_t error_resp;
		};
		uint32_t u32[8];
	}regs; ///de este es de quien reserva luego espacio en la memoria

	sc_event ev_dma_copy;
	void do_dma_trans(tlm::tlm_command cmd, unsigned char *buf,
			sc_dt::uint64 addr, sc_dt::uint64 len);
	void do_dma_copy(void);
	void update_irqs(void);

	virtual void b_transport(tlm::tlm_generic_payload& trans,
					sc_time& delay);
};
