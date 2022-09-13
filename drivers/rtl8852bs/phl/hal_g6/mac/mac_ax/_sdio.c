/******************************************************************************
 *
 * Copyright(c) 2019 Realtek Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 ******************************************************************************/
#include "_sdio.h"

#if MAC_AX_SDIO_SUPPORT
static struct mac_sdio_tbl sdio_tbl;

static u8 r_indir_cmd52_sdio(struct mac_ax_adapter *adapter, u32 adr);
static u8 _r_indir_cmd52_sdio(struct mac_ax_adapter *adapter, u32 adr);
static u32 r_indir_cmd53_sdio(struct mac_ax_adapter *adapter, u32 adr);
static u32 _r_indir_cmd53_sdio(struct mac_ax_adapter *adapter, u32 adr);
static u32 r8_indir_sdio(struct mac_ax_adapter *adapter, u32 adr);
static u32 r16_indir_sdio(struct mac_ax_adapter *adapter, u32 adr);
static u32 r32_indir_sdio(struct mac_ax_adapter *adapter, u32 adr);
static void w_indir_cmd52_sdio(struct mac_ax_adapter *adapter, u32 adr, u32 val,
			       enum sdio_io_size size);
static void w_indir_cmd53_sdio(struct mac_ax_adapter *adapter, u32 adr, u32 val,
			       enum sdio_io_size size);
static void w8_indir_sdio(struct mac_ax_adapter *adapter, u32 adr, u32 val);
static void w16_indir_sdio(struct mac_ax_adapter *adapter, u32 adr, u32 val);
static void w32_indir_sdio(struct mac_ax_adapter *adapter, u32 adr, u32 val);
static u8 pwr_state_chk_sdio(struct mac_ax_adapter *adapter);
static u8 reg_chk_sdio(struct mac_ax_adapter *adapter, u32 adr);
static void chk_rqd_pg_num(struct mac_ax_adapter *adapter,
			   struct mac_ax_sdio_tx_info *tx_info);
static u32 chk_fs_enuf(struct mac_ax_adapter *adapter,
		       struct mac_ax_sdio_tx_info *tx_info);
static void ud_fs(struct mac_ax_adapter *adapter);
static u32 get_pg_size_pow(u32 size);
static u32 tx_allow_data_ch(struct mac_ax_adapter *adapter,
			    struct mac_ax_sdio_tx_info *info);
static u32 tx_allow_fwcmd_ch(struct mac_ax_adapter *adapter,
			     struct mac_ax_sdio_tx_info *info);

static u8 _pltfm_sdio_cmd53_r8(struct mac_ax_adapter *adapter, u32 adr)
{
	u32 dw_adr = adr & 0xFFFFFFFC;
	u8 dw_sh = adr - dw_adr;
	u8 cnt = 0;
	union {
		__le32 dword;
		u8 byte[4];
	} val32 = { 0x00000000 };

	if (is_chip_id(adapter, MAC_AX_CHIP_ID_8852A) ||
	    is_chip_id(adapter, MAC_AX_CHIP_ID_8852B)) {
		if (!(adr >= R_AX_CMAC_FUNC_EN && adr <= R_AX_CMAC_REG_END))
			return PLTFM_SDIO_CMD53_R8(adr);
	} else {
		return PLTFM_SDIO_CMD53_R8(adr);
	}

	while (cnt < MAC_REG_POOL_COUNT) {
		val32.dword = PLTFM_SDIO_CMD53_R32(dw_adr);

		if (le32_to_cpu(val32.dword) != MAC_AX_R32_DEAD)
			break;

		PLTFM_MSG_ERR("[ERR]addr 0x%x = 0xdeadbeef\n", dw_adr);
		PLTFM_SDIO_CMD53_W32(R_AX_CK_EN, CMAC_CLK_ALLEN);
		cnt++;
	}

	return val32.byte[dw_sh];
}

static u16 _pltfm_sdio_cmd53_r16(struct mac_ax_adapter *adapter, u32 adr)
{
	u32 dw_adr = adr & 0xFFFFFFFD;
	u8 dw_sh = (adr - dw_adr) ? 1 : 0;
	u8 cnt = 0;
	union {
		__le32 dword;
		u16 word[2];
	} val32 = { 0x00000000 };

	if (is_chip_id(adapter, MAC_AX_CHIP_ID_8852A) ||
	    is_chip_id(adapter, MAC_AX_CHIP_ID_8852B)) {
		if (!(adr >= R_AX_CMAC_FUNC_EN && adr <= R_AX_CMAC_REG_END))
			return PLTFM_SDIO_CMD53_R16(adr);
	} else {
		return PLTFM_SDIO_CMD53_R16(adr);
	}

	while (cnt < MAC_REG_POOL_COUNT) {
		val32.dword = PLTFM_SDIO_CMD53_R32(dw_adr);

		if (le32_to_cpu(val32.dword) != MAC_AX_R32_DEAD)
			break;

		PLTFM_MSG_ERR("[ERR]addr 0x%x = 0xdeadbeef\n", dw_adr);
		PLTFM_SDIO_CMD53_W32(R_AX_CK_EN, CMAC_CLK_ALLEN);
		cnt++;
	}

	return val32.word[dw_sh];
}

static u32 _pltfm_sdio_cmd53_r32(struct mac_ax_adapter *adapter, u32 adr)
{
	u8 cnt = 0;
	__le32 dword = 0x00000000;

	if (is_chip_id(adapter, MAC_AX_CHIP_ID_8852A) ||
	    is_chip_id(adapter, MAC_AX_CHIP_ID_8852B)) {
		if (!(adr >= R_AX_CMAC_FUNC_EN && adr <= R_AX_CMAC_REG_END))
			return PLTFM_SDIO_CMD53_R32(adr);
	} else {
		return PLTFM_SDIO_CMD53_R32(adr);
	}

	while (cnt < MAC_REG_POOL_COUNT) {
		dword = PLTFM_SDIO_CMD53_R32(adr);

		if (le32_to_cpu(dword) != MAC_AX_R32_DEAD)
			break;

		PLTFM_MSG_ERR("[ERR]addr 0x%x = 0xdeadbeef\n", adr);
		PLTFM_SDIO_CMD53_W32(R_AX_CK_EN, CMAC_CLK_ALLEN);
		cnt++;
	}

	return dword;
}

u8 reg_read8_sdio(struct mac_ax_adapter *adapter, u32 adr)
{
	u8 val8;
	u8 pwr_state, reg_domain;
	enum mac_ax_sdio_4byte_mode cmd53_4byte = adapter->sdio_info.sdio_4byte;

	pwr_state = pwr_state_chk_sdio(adapter);
	reg_domain = reg_chk_sdio(adapter, adr);

	if (reg_domain == SDIO_REG_LOCAL) {
		val8 = PLTFM_SDIO_CMD52_R8(adr);
	} else if (reg_domain == SDIO_REG_WLAN_PLTFM) {
		val8 = (u8)r_indir_sdio(adapter, adr, SDIO_IO_BYTE);
	} else {
		if (pwr_state == SDIO_PWR_OFF)
			val8 = (u8)r_indir_sdio(adapter, adr, SDIO_IO_BYTE);
		else if (cmd53_4byte == MAC_AX_SDIO_4BYTE_MODE_DISABLE)
			val8 = _pltfm_sdio_cmd53_r8(adapter, adr);
		else
			val8 = (u8)(_pltfm_sdio_cmd53_r32(adapter, adr) & 0xFF);
	}

	return val8;
}

void reg_write8_sdio(struct mac_ax_adapter *adapter, u32 adr, u8 val)
{
	u8 reg_domain;

	reg_domain = reg_chk_sdio(adapter, adr);

	if (reg_domain == SDIO_REG_LOCAL)
		PLTFM_SDIO_CMD52_W8(adr, val);
	else
		w_indir_sdio(adapter, adr, val, SDIO_IO_BYTE);
}

u16 reg_read16_sdio(struct mac_ax_adapter *adapter, u32 adr)
{
	u8 pwr_state, reg_domain;
	enum mac_ax_sdio_4byte_mode sdio_4byte = adapter->sdio_info.sdio_4byte;
	union {
		__le16 word;
		u8 byte[2];
	} value16 = { 0x0000 };

	pwr_state = pwr_state_chk_sdio(adapter);
	reg_domain = reg_chk_sdio(adapter, adr);

	if ((adr & (2 - 1)) == 0) {
		if (pwr_state == SDIO_PWR_ON &&
		    sdio_4byte == MAC_AX_SDIO_4BYTE_MODE_DISABLE)
			return _pltfm_sdio_cmd53_r16(adapter, adr);
		if (reg_domain == SDIO_REG_LOCAL) {
			value16.byte[0] = PLTFM_SDIO_CMD52_R8(adr);
			value16.byte[1] = PLTFM_SDIO_CMD52_R8(adr + 1);
			return le16_to_cpu(value16.word);
		}
		if (pwr_state == SDIO_PWR_OFF ||
		    reg_domain == SDIO_REG_WLAN_PLTFM)
			return (u16)r_indir_sdio(adapter, adr, SDIO_IO_WORD);
		return (u16)(_pltfm_sdio_cmd53_r32(adapter, adr) & 0xFFFF);
	}

	if (reg_domain == SDIO_REG_LOCAL) {
		value16.byte[0] = PLTFM_SDIO_CMD52_R8(adr);
		value16.byte[1] = PLTFM_SDIO_CMD52_R8(adr + 1);
		return le16_to_cpu(value16.word);
	}
	value16.byte[0] = (u8)r_indir_sdio(adapter, adr, SDIO_IO_BYTE);
	value16.byte[1] = (u8)r_indir_sdio(adapter, adr + 1, SDIO_IO_BYTE);
	return le16_to_cpu(value16.word);
}

void reg_write16_sdio(struct mac_ax_adapter *adapter, u32 adr, u16 val)
{
	u8 pwr_state, reg_domain;
	enum mac_ax_sdio_4byte_mode sdio_4byte = adapter->sdio_info.sdio_4byte;

	pwr_state = pwr_state_chk_sdio(adapter);
	reg_domain = reg_chk_sdio(adapter, adr);

	if ((adr & (2 - 1)) == 0) {
		if (pwr_state == SDIO_PWR_ON &&
		    sdio_4byte == MAC_AX_SDIO_4BYTE_MODE_DISABLE) {
			PLTFM_SDIO_CMD53_W16(adr, val);
		} else if (reg_domain == SDIO_REG_WLAN_REG ||
			   reg_domain == SDIO_REG_WLAN_PLTFM) {
			w_indir_sdio(adapter, adr, val, SDIO_IO_WORD);
		} else {
			PLTFM_SDIO_CMD52_W8(adr, (u8)(val & 0xFF));
			PLTFM_SDIO_CMD52_W8(adr + 1,
					    (u8)((val & 0xFF00) >> 8));
		}
	} else {
		if (reg_domain == SDIO_REG_LOCAL) {
			PLTFM_SDIO_CMD52_W8(adr, (u8)(val & 0xFF));
			PLTFM_SDIO_CMD52_W8(adr + 1,
					    (u8)((val & 0xFF00) >> 8));
		} else {
			w_indir_sdio(adapter, adr, (u8)(val & 0xFF),
				     SDIO_IO_BYTE);
			w_indir_sdio(adapter, adr + 1,
				     (u8)((val & 0xFF00) >> 8), SDIO_IO_BYTE);
		}
	}
}

u32 reg_read32_sdio(struct mac_ax_adapter *adapter, u32 adr)
{
	u8 pwr_state, reg_domain;
	union {
		__le32 dword;
		u8 byte[4];
	} value32 = { 0x00000000 };

	pwr_state = pwr_state_chk_sdio(adapter);
	reg_domain = reg_chk_sdio(adapter, adr);

	if ((adr & (4 - 1)) == 0) {
		if (pwr_state == SDIO_PWR_OFF && reg_domain == SDIO_REG_LOCAL) {
			value32.byte[0] = PLTFM_SDIO_CMD52_R8(adr);
			value32.byte[1] = PLTFM_SDIO_CMD52_R8(adr + 1);
			value32.byte[2] = PLTFM_SDIO_CMD52_R8(adr + 2);
			value32.byte[3] = PLTFM_SDIO_CMD52_R8(adr + 3);
			return le32_to_cpu(value32.dword);
		}
		if (pwr_state == SDIO_PWR_ON &&
		    (reg_domain == SDIO_REG_LOCAL ||
		    reg_domain == SDIO_REG_WLAN_REG))
			return _pltfm_sdio_cmd53_r32(adapter, adr);
		return r_indir_sdio(adapter, adr, SDIO_IO_DWORD);
	}

	if (reg_domain == SDIO_REG_LOCAL) {
		value32.byte[0] = PLTFM_SDIO_CMD52_R8(adr);
		value32.byte[1] = PLTFM_SDIO_CMD52_R8(adr + 1);
		value32.byte[2] = PLTFM_SDIO_CMD52_R8(adr + 2);
		value32.byte[3] = PLTFM_SDIO_CMD52_R8(adr + 3);
		return le32_to_cpu(value32.dword);
	}
	value32.byte[0] = (u8)r_indir_sdio(adapter, adr, SDIO_IO_BYTE);
	value32.byte[1] = (u8)r_indir_sdio(adapter, adr + 1, SDIO_IO_BYTE);
	value32.byte[2] = (u8)r_indir_sdio(adapter, adr + 2, SDIO_IO_BYTE);
	value32.byte[3] = (u8)r_indir_sdio(adapter, adr + 3, SDIO_IO_BYTE);
	return le32_to_cpu(value32.dword);
}

void reg_write32_sdio(struct mac_ax_adapter *adapter, u32 adr, u32 val)
{
	u8 pwr_state, reg_domain;

	pwr_state = pwr_state_chk_sdio(adapter);
	reg_domain = reg_chk_sdio(adapter, adr);

	if ((adr & (4 - 1)) == 0) {
		if (pwr_state == SDIO_PWR_OFF && reg_domain == SDIO_REG_LOCAL) {
			PLTFM_SDIO_CMD52_W8(adr, (u8)(val & 0xFF));
			PLTFM_SDIO_CMD52_W8(adr + 1, (u8)((val >> 8) & 0xFF));
			PLTFM_SDIO_CMD52_W8(adr + 2, (u8)((val >> 16) & 0xFF));
			PLTFM_SDIO_CMD52_W8(adr + 3, (u8)((val >> 24) & 0xFF));
		} else if (pwr_state == SDIO_PWR_ON &&
			   (reg_domain == SDIO_REG_LOCAL ||
			   reg_domain == SDIO_REG_WLAN_REG)) {
			PLTFM_SDIO_CMD53_W32(adr, val);
		} else {
			w_indir_sdio(adapter, adr, val, SDIO_IO_DWORD);
		}
	} else {
		if (reg_domain == SDIO_REG_LOCAL) {
			PLTFM_SDIO_CMD52_W8(adr, (u8)(val & 0xFF));
			PLTFM_SDIO_CMD52_W8(adr + 1, (u8)((val >> 8) & 0xFF));
			PLTFM_SDIO_CMD52_W8(adr + 2, (u8)((val >> 16) & 0xFF));
			PLTFM_SDIO_CMD52_W8(adr + 3, (u8)((val >> 24) & 0xFF));
		} else {
			w_indir_sdio(adapter, adr, (u8)(val & 0xFF),
				     SDIO_IO_BYTE);
			w_indir_sdio(adapter, adr + 1,
				     (u8)((val >> 8) & 0xFF), SDIO_IO_BYTE);
			w_indir_sdio(adapter, adr + 2,
				     (u8)((val >> 16) & 0xFF), SDIO_IO_BYTE);
			w_indir_sdio(adapter, adr + 3,
				     (u8)((val >> 24) & 0xFF), SDIO_IO_BYTE);
		}
	}
}

u32 reg_read_n_sdio(struct mac_ax_adapter *adapter, u32 adr, u32 size, u8 *val)
{
	u8 *r_val = NULL;
	u32 r_size;
	u8 pwr_state, reg_domain;
	enum mac_ax_sdio_4byte_mode cmd53_4byte = adapter->sdio_info.sdio_4byte;

	pwr_state = pwr_state_chk_sdio(adapter);
	reg_domain = reg_chk_sdio(adapter, adr);

	if (reg_domain != SDIO_REG_LOCAL) {
		PLTFM_MSG_ERR("[ERR]adr 0x%x\n", adr);
		return MACBADDR;
	}

	if (pwr_state == SDIO_PWR_OFF) {
		PLTFM_MSG_ERR("[ERR]power off\n");
		return MACPWRSTAT;
	}

	if (cmd53_4byte == MAC_AX_SDIO_4BYTE_MODE_RW && (size & 0x03) != 0) {
		PLTFM_MSG_WARN("[WARN]reg_rn !align,addr 0x%x,siz %d\n", adr,
			       size);
		r_size = size - (size & 0x03) + 4;
		r_val = (u8 *)PLTFM_MALLOC(r_size);
		if (!r_val) {
			PLTFM_MSG_ERR("[ERR]malloc!!\n");
			return MACBUFALLOC;
		}
		PLTFM_MEMSET(r_val, 0x00, r_size);
		PLTFM_SDIO_CMD53_RN(adr, r_size, r_val);
		PLTFM_MEMCPY(val, r_val, size);
		PLTFM_FREE(r_val, r_size);
	} else {
		PLTFM_SDIO_CMD53_RN(adr, size, val);
	}

	return MACSUCCESS;
}

u32 tx_allow_sdio(struct mac_ax_adapter *adapter,
		  struct mac_ax_sdio_tx_info *info)
{
	u32 ret;

	if (info->ch_dma != MAC_AX_DMA_H2C)
		ret = tx_allow_data_ch(adapter, info);
	else
		ret = tx_allow_fwcmd_ch(adapter, info);

	if (ret != MACSUCCESS)
		return ret;

	return MACSUCCESS;
}

u32 tx_cmd_addr_sdio(struct mac_ax_adapter *adapter,
		     struct mac_ax_sdio_tx_info *info, u32 *cmd_addr)
{
	struct mac_ax_sdio_info *sdio_info = &adapter->sdio_info;
	u16 block_size = sdio_info->block_size;
	u32 len_unit1, len_unit8, val32, size;
	enum sdio_tx_byte_cnt byte_cnt;
	enum mac_ax_sdio_opn_mode opn_mode = sdio_info->opn_mode;

	size = info->total_size;

	if (size == 0) {
		PLTFM_MSG_ERR("size is 0!!\n");
		return MACBUFSZ;
	}

	if (info->ch_dma < 13)
		*cmd_addr = (info->ch_dma << SDIO_CMD_ADDR_TXFF_SHIFT) |
			     SDIO_TX_BASE;
	else
		return MACTXCHDMA;

	len_unit8 = (size >> 3) + ((size & (8 - 1)) ? 1 : 0);
	len_unit1 = (len_unit8 << 3);

	switch (sdio_info->tx_mode) {
	case MAC_AX_SDIO_TX_MODE_AGG:
		byte_cnt = SDIO_TX_AGG_8_BYTE_CNT;
		break;
	case MAC_AX_SDIO_TX_MODE_DUMMY_BLOCK:
		byte_cnt = SDIO_TX_DUMMY_4_BYTE_CNT;
		break;
	case MAC_AX_SDIO_TX_MODE_DUMMY_AUTO:
		if (len_unit1 == SDIO_BYTE_MODE_SIZE_MAX) {
			if (opn_mode == MAC_AX_SDIO_OPN_MODE_BYTE) {
				byte_cnt = SDIO_TX_AGG_8_BYTE_CNT;
			} else if (opn_mode == MAC_AX_SDIO_OPN_MODE_BLOCK) {
				byte_cnt = SDIO_TX_DUMMY_4_BYTE_CNT;
			} else {
				PLTFM_MSG_ERR("[ERR]opn_mode is unknown\n");
				return MACSDIOOPNMODE;
			}
		} else if (len_unit1 >= block_size) {
			byte_cnt = SDIO_TX_DUMMY_4_BYTE_CNT;
		} else if (len_unit1 < SDIO_BYTE_MODE_SIZE_MAX) {
			byte_cnt = SDIO_TX_AGG_8_BYTE_CNT;
		} else {
			byte_cnt = SDIO_TX_DUMMY_4_BYTE_CNT;
		}
		break;
	default:
		PLTFM_MSG_ERR("[ERR]tx_mode is undefined\n");
		return MACSDIOTXMODE;
	}

	if (byte_cnt == SDIO_TX_AGG_8_BYTE_CNT) {
		*cmd_addr |= len_unit8 & SDIO_8BYTE_LEN_MASK;
	} else if (byte_cnt == SDIO_TX_DUMMY_4_BYTE_CNT) {
		val32 = len_unit1 & (block_size - 1);
		if (val32)
			val32 = block_size - val32;
		val32 = (val32 >> 2) & SDIO_4BYTE_LEN_MASK;
		*cmd_addr |= (val32 << 1) | sdio_info->tx_seq;
		sdio_info->tx_seq = ~sdio_info->tx_seq & 0x01;
	}

	//cmd_len = adapter->sdio_info.tx_align_size;

	return MACSUCCESS;
}

u32 sdio_pre_init(struct mac_ax_adapter *adapter, void *param)
{
	u16 val16;
	u32 val32;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	val32 = MAC_REG_R32(R_AX_HCI_OPT_CTRL);
	MAC_REG_W32(R_AX_HCI_OPT_CTRL, val32 | B_AX_SDIO_DATA_PAD_SMT);

	val32 = MAC_REG_R32(R_AX_SDIO_TX_CTRL) & ~(B_AX_CMD53_TX_FORMAT);
	MAC_REG_W32(R_AX_SDIO_TX_CTRL, val32 | B_AX_RXINT_READ_MASK_DIS);
	adapter->sdio_info.tx_mode = MAC_AX_SDIO_TX_MODE_AGG;
	adapter->sdio_info.tx_seq = 1;

	val16 = MAC_REG_R16(R_AX_SDIO_BUS_CTRL);
	MAC_REG_W16(R_AX_SDIO_BUS_CTRL, val16 | B_AX_EN_RPT_TXCRC);

	return MACSUCCESS;
}

u32 sdio_init(struct mac_ax_adapter *adapter, void *param)
{
	u32 val32;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	if (adapter->hw_info->intf != MAC_AX_INTF_SDIO)
		return MACINTF;

	val32 = MAC_REG_R32(R_AX_RXDMA_SETTING);
	MAC_REG_W32(R_AX_RXDMA_SETTING, val32 & ~B_AX_PLE_BURST_READ);

	return MACSUCCESS;
}

u32 sdio_deinit(struct mac_ax_adapter *adapter, void *param)
{
	return MACSUCCESS;
}

u32 r_indir_sdio(struct mac_ax_adapter *adapter, u32 adr,
		 enum sdio_io_size size)
{
	u32 value32 = 0;

	PLTFM_MUTEX_LOCK(&sdio_tbl.lock);

	switch (size) {
	case SDIO_IO_BYTE:
		value32 = r8_indir_sdio(adapter, adr);
		break;
	case SDIO_IO_WORD:
		value32 = r16_indir_sdio(adapter, adr);
		break;
	case SDIO_IO_DWORD:
		value32 = r32_indir_sdio(adapter, adr);
		break;
	default:
		PLTFM_MSG_ERR("[ERR]invalid IO size\n");
		break;
	}

	PLTFM_MUTEX_UNLOCK(&sdio_tbl.lock);

	return value32;
}

void w_indir_sdio(struct mac_ax_adapter *adapter, u32 adr, u32 val,
		  enum sdio_io_size size)
{
	PLTFM_MUTEX_LOCK(&sdio_tbl.lock);

	switch (size) {
	case SDIO_IO_BYTE:
		w8_indir_sdio(adapter, adr, val);
		break;
	case SDIO_IO_WORD:
		w16_indir_sdio(adapter, adr, val);
		break;
	case SDIO_IO_DWORD:
		w32_indir_sdio(adapter, adr, val);
		break;
	default:
		PLTFM_MSG_ERR("[ERR]invalid IO size\n");
		break;
	}

	PLTFM_MUTEX_UNLOCK(&sdio_tbl.lock);
}

u32 set_info_sdio(struct mac_ax_adapter *adapter, struct mac_ax_sdio_info *info)
{
	adapter->sdio_info.spec_ver = info->spec_ver;
	adapter->sdio_info.block_size = info->block_size;
	adapter->sdio_info.sdio_4byte = info->sdio_4byte;
	adapter->sdio_info.opn_mode = info->opn_mode;

	return MACSUCCESS;
}

u32 tx_mode_cfg_sdio(struct mac_ax_adapter *adapter,
		     enum mac_ax_sdio_tx_mode mode)
{
	u16 val16;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	val16 = MAC_REG_R16(R_AX_SDIO_TX_CTRL);

	if (mode == MAC_AX_SDIO_TX_MODE_AGG) {
		MAC_REG_W16(R_AX_SDIO_TX_CTRL, val16 & ~(B_AX_CMD53_TX_FORMAT));
	} else if (mode == MAC_AX_SDIO_TX_MODE_DUMMY_BLOCK ||
		   mode == MAC_AX_SDIO_TX_MODE_DUMMY_AUTO) {
		if ((val16 & B_AX_CMD53_W_MIX) == 0) {
			MAC_REG_W16(R_AX_SDIO_TX_CTRL,
				    val16 | B_AX_CMD53_TX_FORMAT);
		} else {
			if ((val16 & B_AX_CMD53_TX_FORMAT) == 0)
				return MACSDIOMIXMODE;
			else
				return MACSDIOSEQERR;
		}
	} else {
		PLTFM_MSG_ERR("[ERR]sdio tx mode = %d\n", mode);
		return MACNOITEM;
	}

	adapter->sdio_info.tx_mode = mode;

	return MACSUCCESS;
}

void rx_agg_cfg_sdio(struct mac_ax_adapter *adapter,
		     struct mac_ax_rx_agg_cfg *cfg)
{
	u8 size;
	u8 timeout;
	u8 agg_en;
	u8 pkt_num;
	u32 val32;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	if (cfg->mode == MAC_AX_RX_AGG_MODE_DMA)
		agg_en = 1;
	else
		agg_en = 0;

	if (cfg->thold.drv_define == 0) {
		size = 0xFF;
		timeout = 0x01;
		pkt_num = 0;
	} else {
		size = cfg->thold.size;
		timeout = cfg->thold.timeout;
		pkt_num = cfg->thold.pkt_num;
	}

	val32 = MAC_REG_R32(R_AX_RXAGG_0);
	MAC_REG_W32(R_AX_RXAGG_0, (agg_en ? B_AX_RXAGG_EN : 0) |
		    B_AX_RXAGG_DMA_STORE | (val32 & B_AX_RXAGG_SW_EN) |
		    SET_WORD(pkt_num, B_AX_RXAGG_PKTNUM_TH) |
		    SET_WORD(timeout, B_AX_RXAGG_TIMEOUT_TH) |
		    SET_WORD(size, B_AX_RXAGG_LEN_TH));
}

u32 tx_agg_cfg_sdio(struct mac_ax_adapter *adapter,
		    struct mac_ax_sdio_txagg_cfg *cfg)
{
	u8 i;
	u8 flag = 0;
	u16 align_size = cfg->align_size;
	u8 enable = cfg->en;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	if ((align_size & 0xF000) != 0) {
		PLTFM_MSG_ERR("[ERR]out of range\n");
		return MACFUNCINPUT;
	}

	for (i = 3; i <= 11; i++) {
		if (align_size == 1 << i) {
			flag = 1;
			break;
		}
	}

	if (flag == 0) {
		PLTFM_MSG_ERR("[ERR]not 2^3 ~ 2^11\n");
		return MACFUNCINPUT;
	}

	if (enable) {
		MAC_REG_W32(R_AX_TXAGG_ALIGN_CFG,
			    align_size | B_AX_TXAGG_ALIGN_SIZE_EN);
	} else {
		MAC_REG_W32(R_AX_TXAGG_ALIGN_CFG,
			    align_size & ~B_AX_TXAGG_ALIGN_SIZE_EN);
		align_size = 8;
	}

	adapter->sdio_info.tx_align_size = align_size;

	return MACSUCCESS;
}

void aval_page_cfg_sdio(struct mac_ax_adapter *adapter,
			struct mac_ax_aval_page_cfg *cfg)
{
	u32 val32;
	struct mac_sdio_ch_thr ch_thr;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	switch (cfg->ch_dma) {
	case 0:
		ch_thr.thr = R_AX_ACH0_THR;
		ch_thr.intrpt_en = B_AX_ACH0_INTRPT_EN;
		ch_thr.wp_sh = B_AX_ACH0_THR_WP_SH;
		ch_thr.wp_msk = B_AX_ACH0_THR_WP_MSK;
		ch_thr.wd_sh = B_AX_ACH0_THR_WD_SH;
		ch_thr.wd_msk = B_AX_ACH0_THR_WD_MSK;
		break;
	case 1:
		ch_thr.thr = R_AX_ACH1_THR;
		ch_thr.intrpt_en = B_AX_ACH1_INTRPT_EN;
		ch_thr.wp_sh = B_AX_ACH1_THR_WP_SH;
		ch_thr.wp_msk = B_AX_ACH1_THR_WP_MSK;
		ch_thr.wd_sh = B_AX_ACH1_THR_WD_SH;
		ch_thr.wd_msk = B_AX_ACH1_THR_WD_MSK;
		break;
	case 2:
		ch_thr.thr = R_AX_ACH2_THR;
		ch_thr.intrpt_en = B_AX_ACH2_INTRPT_EN;
		ch_thr.wp_sh = B_AX_ACH2_THR_WP_SH;
		ch_thr.wp_msk = B_AX_ACH2_THR_WP_MSK;
		ch_thr.wd_sh = B_AX_ACH2_THR_WD_SH;
		ch_thr.wd_msk = B_AX_ACH2_THR_WD_MSK;
		break;
	case 3:
		ch_thr.thr = R_AX_ACH3_THR;
		ch_thr.intrpt_en = B_AX_ACH3_INTRPT_EN;
		ch_thr.wp_sh = B_AX_ACH3_THR_WP_SH;
		ch_thr.wp_msk = B_AX_ACH3_THR_WP_MSK;
		ch_thr.wd_sh = B_AX_ACH3_THR_WD_SH;
		ch_thr.wd_msk = B_AX_ACH3_THR_WD_MSK;
		break;
	case 4:
		ch_thr.thr = R_AX_ACH4_THR;
		ch_thr.intrpt_en = B_AX_ACH4_INTRPT_EN;
		ch_thr.wp_sh = B_AX_ACH4_THR_WP_SH;
		ch_thr.wp_msk = B_AX_ACH4_THR_WP_MSK;
		ch_thr.wd_sh = B_AX_ACH4_THR_WD_SH;
		ch_thr.wd_msk = B_AX_ACH4_THR_WD_MSK;
		break;
	case 5:
		ch_thr.thr = R_AX_ACH5_THR;
		ch_thr.intrpt_en = B_AX_ACH5_INTRPT_EN;
		ch_thr.wp_sh = B_AX_ACH5_THR_WP_SH;
		ch_thr.wp_msk = B_AX_ACH5_THR_WP_MSK;
		ch_thr.wd_sh = B_AX_ACH5_THR_WD_SH;
		ch_thr.wd_msk = B_AX_ACH5_THR_WD_MSK;
		break;
	case 6:
		ch_thr.thr = R_AX_ACH6_THR;
		ch_thr.intrpt_en = B_AX_ACH6_INTRPT_EN;
		ch_thr.wp_sh = B_AX_ACH6_THR_WP_SH;
		ch_thr.wp_msk = B_AX_ACH6_THR_WP_MSK;
		ch_thr.wd_sh = B_AX_ACH6_THR_WD_SH;
		ch_thr.wd_msk = B_AX_ACH6_THR_WD_MSK;
		break;
	case 7:
		ch_thr.thr = R_AX_ACH7_THR;
		ch_thr.intrpt_en = B_AX_ACH7_INTRPT_EN;
		ch_thr.wp_sh = B_AX_ACH7_THR_WP_SH;
		ch_thr.wp_msk = B_AX_ACH7_THR_WP_MSK;
		ch_thr.wd_sh = B_AX_ACH7_THR_WD_SH;
		ch_thr.wd_msk = B_AX_ACH7_THR_WD_MSK;
		break;
	case 8:
		ch_thr.thr = R_AX_CH8_THR;
		ch_thr.intrpt_en = B_AX_CH8_INTRPT_EN;
		ch_thr.wp_sh = B_AX_CH8_THR_WP_SH;
		ch_thr.wp_msk = B_AX_CH8_THR_WP_MSK;
		ch_thr.wd_sh = B_AX_CH8_THR_WD_SH;
		ch_thr.wd_msk = B_AX_CH8_THR_WD_MSK;
		break;
	case 9:
		ch_thr.thr = R_AX_CH9_THR;
		ch_thr.intrpt_en = B_AX_CH9_INTRPT_EN;
		ch_thr.wp_sh = B_AX_CH9_THR_WP_SH;
		ch_thr.wp_msk = B_AX_CH9_THR_WP_MSK;
		ch_thr.wd_sh = B_AX_CH9_THR_WD_SH;
		ch_thr.wd_msk = B_AX_CH9_THR_WD_MSK;
		break;
	case 10:
		ch_thr.thr = R_AX_CH10_THR;
		ch_thr.intrpt_en = B_AX_CH10_INTRPT_EN;
		ch_thr.wp_sh = B_AX_CH10_THR_WP_SH;
		ch_thr.wp_msk = B_AX_CH10_THR_WP_MSK;
		ch_thr.wd_sh = B_AX_CH10_THR_WD_SH;
		ch_thr.wd_msk = B_AX_CH10_THR_WD_MSK;
		break;
	case 11:
		ch_thr.thr = R_AX_CH11_THR;
		ch_thr.intrpt_en = B_AX_CH11_INTRPT_EN;
		ch_thr.wp_sh = B_AX_CH11_THR_WP_SH;
		ch_thr.wp_msk = B_AX_CH11_THR_WP_MSK;
		ch_thr.wd_sh = B_AX_CH11_THR_WD_SH;
		ch_thr.wd_msk = B_AX_CH11_THR_WD_MSK;
		break;
	default:
		PLTFM_MSG_ERR("[ERR]invalid channel number\n");
		return;
	}

	val32 = ((cfg->thold_wd & ch_thr.wd_msk) << ch_thr.wd_sh) |
		((cfg->thold_wp & ch_thr.wp_msk) << ch_thr.wp_sh);
	if (cfg->en)
		val32 |= ch_thr.intrpt_en;
	else
		val32 &= ~ch_thr.intrpt_en;
	MAC_REG_W32(ch_thr.thr, val32);
}

u32 get_int_latency_sdio(struct mac_ax_adapter *adapter)
{
	u32 free_cnt, free_cnt2;
	u32 int_start;
	u32 int_latency = 0;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	int_start = MAC_REG_R32(R_AX_SDIO_MONITOR);
	free_cnt = MAC_REG_R32(R_AX_FREERUN_CNT_LOW);
	free_cnt2 = MAC_REG_R32(R_AX_FREERUN_CNT_LOW);
	int_latency = free_cnt - int_start - (free_cnt2 - free_cnt);

	return int_latency;
}

u32 get_clk_cnt_sdio(struct mac_ax_adapter *adapter, u32 *cnt)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32;

	val32 = MAC_REG_R32(R_AX_SDIO_MONITOR_2);
	if (GET_FIELD(val32, B_AX_SDIO_CLK_MONITOR) == 0)
		*cnt = GET_FIELD(val32, B_AX_SDIO_CLK_CNT);
	else
		return MACNOITEM;

	return MACSUCCESS;
}

u32 set_wt_cfg_sdio(struct mac_ax_adapter *adapter, u8 en)
{
	u32 reg = R_AX_SDIO_MONITOR_2 + 2;

	if (en)
		PLTFM_SDIO_CMD52_W8(reg, BIT(7));
	else
		PLTFM_SDIO_CMD52_W8(reg, 0);

	return MACSUCCESS;
}

u32 set_clk_mon_sdio(struct mac_ax_adapter *adapter,
		     struct mac_ax_sdio_clk_mon_cfg *cfg)
{
	u32 reg = R_AX_SDIO_MONITOR_2 + 2;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	set_wt_cfg_sdio(adapter, 0);

	switch (cfg->mon) {
	case MAC_AX_SDIO_CLK_MON_SHORT:
		PLTFM_SDIO_CMD52_W8(reg, BIT(5));
		break;
	case MAC_AX_SDIO_CLK_MON_LONG:
		PLTFM_SDIO_CMD52_W8(reg, BIT(6));
		break;
	case MAC_AX_SDIO_CLK_MON_USER_DEFINE:
		MAC_REG_W32(R_AX_SDIO_MONITOR_2,
			    cfg->cycle & B_AX_SDIO_CLK_CNT_MSK);
		PLTFM_SDIO_CMD52_W8(reg, BIT(5) | BIT(6));
		break;
	default:
		break;
	}

	return MACSUCCESS;
}

u32 sdio_tbl_init(struct mac_ax_adapter *adapter)
{
	PLTFM_MUTEX_INIT(&sdio_tbl.lock);

	return MACSUCCESS;
}

u32 sdio_tbl_exit(struct mac_ax_adapter *adapter)
{
	PLTFM_MUTEX_DEINIT(&sdio_tbl.lock);

	return MACSUCCESS;
}

u32 leave_suspend_sdio(struct mac_ax_adapter *adapter)
{
	u8 val8;
	u32 cnt;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	val8 = MAC_REG_R8(R_AX_SDIO_BUS_CTRL + 2);
	MAC_REG_W8(R_AX_SDIO_BUS_CTRL + 2, val8 & ~(BIT(0)));

	cnt = 10000;
	while (!(MAC_REG_R8(R_AX_SDIO_BUS_CTRL + 2) & BIT(1))) {
		cnt--;
		if (cnt == 0)
			return MACLSUS;
	}

	val8 = MAC_REG_R8(R_AX_HCI_OPT_CTRL + 2);
	if (adapter->sdio_info.spec_ver == MAC_AX_SDIO_SPEC_VER_3_00)
		MAC_REG_W8(R_AX_HCI_OPT_CTRL + 2, val8 | BIT(2));
	else
		MAC_REG_W8(R_AX_HCI_OPT_CTRL + 2, val8 & ~(BIT(2)));

	return MACSUCCESS;
}

u32 sdio_pwr_switch(void *vadapter,
		    u8 pre_switch, u8 on)
{
	struct mac_ax_adapter *adapter = (struct mac_ax_adapter *)vadapter;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	if (pre_switch == PWR_PRE_SWITCH) {
		adapter->sdio_info.rpwm_bak = MAC_REG_R32(R_AX_SDIO_HIMR);
		MAC_REG_W32(R_AX_SDIO_HIMR, 0);
		adapter->mac_pwr_info.pwr_seq_proc = 1;
	} else if (pre_switch == PWR_POST_SWITCH) {
		if (on)
			adapter->sdio_info.tx_seq = 1;
		adapter->mac_pwr_info.pwr_seq_proc = 0;
	} else if (pre_switch == PWR_END_SWITCH) {
		MAC_REG_W32(R_AX_SDIO_HIMR, adapter->sdio_info.rpwm_bak);
	}

	return MACSUCCESS;
}

u32 set_sdio_wowlan(struct mac_ax_adapter *adapter, enum mac_ax_wow_ctrl w_c)
{
	return MACSUCCESS;
}

static u8 r_indir_cmd52_sdio(struct mac_ax_adapter *adapter, u32 adr)
{
	u8 count = 0;
	u32 dw_adr = adr & 0xFFFFFFFC;
	u8 dw_sh = adr & (4 - 1);
	u32 val = CMAC_CLK_ALLEN;

	union {
		__le32 dword;
		u8 byte[4];
	} val32 = { 0x00000000 };

	if (is_chip_id(adapter, MAC_AX_CHIP_ID_8852A) ||
	    is_chip_id(adapter, MAC_AX_CHIP_ID_8852B)) {
		if (!(adr >= R_AX_CMAC_FUNC_EN && adr <= R_AX_CMAC_REG_END))
			return _r_indir_cmd52_sdio(adapter, adr);
	} else {
		return _r_indir_cmd52_sdio(adapter, adr);
	}

	while (count < MAC_REG_POOL_COUNT) {
		val32.byte[0] = _r_indir_cmd52_sdio(adapter, dw_adr);
		val32.byte[1] =
			PLTFM_SDIO_CMD52_R8(R_AX_SDIO_INDIRECT_DATA + 1);
		val32.byte[2] =
			PLTFM_SDIO_CMD52_R8(R_AX_SDIO_INDIRECT_DATA + 2);
		val32.byte[3] =
			PLTFM_SDIO_CMD52_R8(R_AX_SDIO_INDIRECT_DATA + 3);

		if (le32_to_cpu(val32.dword) != MAC_AX_R32_DEAD)
			break;

		PLTFM_MSG_ERR("[ERR]addr 0x%x = 0xdeadbeef\n", dw_adr);

		PLTFM_SDIO_CMD52_W8(R_AX_CK_EN, (u8)val);
		PLTFM_SDIO_CMD52_W8(R_AX_CK_EN + 1, (u8)(val >> 8));
		PLTFM_SDIO_CMD52_W8(R_AX_CK_EN + 2, (u8)(val >> 16));
		PLTFM_SDIO_CMD52_W8(R_AX_CK_EN + 3, (u8)(val >> 24));

		count++;
	}

	return val32.byte[dw_sh];
}

static u8 _r_indir_cmd52_sdio(struct mac_ax_adapter *adapter, u32 adr)
{
	u8 tmp;
	u32 cnt;

	PLTFM_SDIO_CMD52_W8(R_AX_SDIO_INDIRECT_ADDR, (u8)adr);
	PLTFM_SDIO_CMD52_W8(R_AX_SDIO_INDIRECT_ADDR + 1, (u8)(adr >> 8));
	PLTFM_SDIO_CMD52_W8(R_AX_SDIO_INDIRECT_ADDR + 2, (u8)(adr >> 16));
	PLTFM_SDIO_CMD52_W8(R_AX_SDIO_INDIRECT_ADDR + 3,
			    (u8)((adr | B_AX_INDIRECT_RDY) >> 24));
	PLTFM_SDIO_CMD52_W8(R_AX_SDIO_INDIRECT_CTRL, (u8)B_AX_INDIRECT_REG_R);

	cnt = SDIO_WAIT_CNT;
	do {
		tmp = PLTFM_SDIO_CMD52_R8(R_AX_SDIO_INDIRECT_ADDR + 3);
		cnt--;
	} while (((tmp & BIT(7)) == 0) && (cnt > 0));

	if (((tmp & BIT(7)) == 0) && cnt == 0)
		PLTFM_MSG_ERR("[ERR]sdio indirect CMD52 read\n");

	return PLTFM_SDIO_CMD52_R8(R_AX_SDIO_INDIRECT_DATA);
}

static u32 r_indir_cmd53_sdio(struct mac_ax_adapter *adapter, u32 adr)
{
	u32 dw_adr = adr & 0xFFFFFFFC;
	u8 dw_sh = (adr & (4 - 1)) << 3;
	u8 count = 0;
	u32 dw = 0;

	if (is_chip_id(adapter, MAC_AX_CHIP_ID_8852A) ||
	    is_chip_id(adapter, MAC_AX_CHIP_ID_8852B)) {
		if (!(adr >= R_AX_CMAC_FUNC_EN && adr <= R_AX_CMAC_REG_END))
			return _r_indir_cmd53_sdio(adapter, adr);
	} else {
		return _r_indir_cmd53_sdio(adapter, adr);
	}

	while (count < MAC_REG_POOL_COUNT) {
		dw = _r_indir_cmd53_sdio(adapter, dw_adr);
		if (dw != MAC_AX_R32_DEAD)
			break;
		PLTFM_MSG_ERR("[ERR]addr 0x%x = 0xdeadbeef\n", dw_adr);
		PLTFM_SDIO_CMD53_W32(R_AX_CK_EN, CMAC_CLK_ALLEN);
		count++;
	}

	return (dw >> dw_sh);
}

static u32 _r_indir_cmd53_sdio(struct mac_ax_adapter *adapter, u32 adr)
{
	u8 val[12] = {0};
	u32 cnt, i;

	union {
		__le32 dword;
		u8 byte[4];
	} value32 = { 0x00000000 };

	for (i = 0; i < 3; i++)
		*(val + i) = (u8)(adr >> (i << 3));
	*(val + 3) = (u8)((adr | B_AX_INDIRECT_RDY) >> 24);
	*(val + 8) = (u8)(B_AX_INDIRECT_REG_R);
	PLTFM_SDIO_CMD53_WN(R_AX_SDIO_INDIRECT_ADDR, sizeof(val), val);

	cnt = SDIO_WAIT_CNT;
	do {
		PLTFM_SDIO_CMD53_RN(R_AX_SDIO_INDIRECT_ADDR + 3, 8, val);
		cnt--;
	} while (((val[0] & BIT(7)) == 0) && (cnt > 0));

	if (((val[0] & BIT(7)) == 0) && cnt == 0)
		PLTFM_MSG_ERR("[ERR]sdio indirect CMD53 read\n");

	value32.byte[0] = val[1];
	value32.byte[1] = val[2];
	value32.byte[2] = val[3];
	value32.byte[3] = val[4];

	return le32_to_cpu(value32.dword);
}

static u32 r8_indir_sdio(struct mac_ax_adapter *adapter, u32 adr)
{
	u8 pwr_state;
	union {
		__le32 dword;
		u8 byte[4];
	} val32 = { 0x00000000 };

	pwr_state = pwr_state_chk_sdio(adapter);

	if (pwr_state == SDIO_PWR_OFF) {
		val32.byte[0] = r_indir_cmd52_sdio(adapter, adr);
		return le32_to_cpu(val32.dword);
	}

	return r_indir_cmd53_sdio(adapter, adr);
}

static u32 r16_indir_sdio(struct mac_ax_adapter *adapter, u32 adr)
{
	u8 pwr_state;
	union {
		__le32 dword;
		u8 byte[4];
	} val32 = { 0x00000000 };

	pwr_state = pwr_state_chk_sdio(adapter);

	if (pwr_state == SDIO_PWR_OFF) {
		val32.byte[0] = r_indir_cmd52_sdio(adapter, adr);
		val32.byte[1] =
			PLTFM_SDIO_CMD52_R8(R_AX_SDIO_INDIRECT_DATA + 1);
		return  le32_to_cpu(val32.dword);
	}

	return r_indir_cmd53_sdio(adapter, adr);
}

static u32 r32_indir_sdio(struct mac_ax_adapter *adapter, u32 adr)
{
	u8 pwr_state;
	union {
		__le32 dword;
		u8 byte[4];
	} val32 = { 0x00000000 };

	pwr_state = pwr_state_chk_sdio(adapter);

	if (pwr_state == SDIO_PWR_OFF) {
		val32.byte[0] = r_indir_cmd52_sdio(adapter, adr);
		val32.byte[1] =
			PLTFM_SDIO_CMD52_R8(R_AX_SDIO_INDIRECT_DATA + 1);
		val32.byte[2] =
			PLTFM_SDIO_CMD52_R8(R_AX_SDIO_INDIRECT_DATA + 2);
		val32.byte[3] =
			PLTFM_SDIO_CMD52_R8(R_AX_SDIO_INDIRECT_DATA + 3);
		return le32_to_cpu(val32.dword);
	}

	return r_indir_cmd53_sdio(adapter, adr);
}

static void w_indir_cmd52_sdio(struct mac_ax_adapter *adapter, u32 adr, u32 val,
			       enum sdio_io_size size)
{
	u8 tmp;
	u32 cnt;

	PLTFM_SDIO_CMD52_W8(R_AX_SDIO_INDIRECT_ADDR, (u8)adr);
	PLTFM_SDIO_CMD52_W8(R_AX_SDIO_INDIRECT_ADDR + 1, (u8)(adr >> 8));
	PLTFM_SDIO_CMD52_W8(R_AX_SDIO_INDIRECT_ADDR + 2, (u8)(adr >> 16));
	PLTFM_SDIO_CMD52_W8(R_AX_SDIO_INDIRECT_ADDR + 3,
			    (u8)((adr | B_AX_INDIRECT_RDY) >> 24));
	switch (size) {
	case SDIO_IO_BYTE:
		PLTFM_SDIO_CMD52_W8(R_AX_SDIO_INDIRECT_DATA, (u8)val);
		PLTFM_SDIO_CMD52_W8(R_AX_SDIO_INDIRECT_CTRL,
				    (u8)(B_AX_INDIRECT_REG_W));
		break;
	case SDIO_IO_WORD:
		PLTFM_SDIO_CMD52_W8(R_AX_SDIO_INDIRECT_DATA, (u8)val);
		PLTFM_SDIO_CMD52_W8(R_AX_SDIO_INDIRECT_DATA + 1,
				    (u8)(val >> 8));
		PLTFM_SDIO_CMD52_W8(R_AX_SDIO_INDIRECT_CTRL,
				    (u8)(B_AX_INDIRECT_REG_W | 0x1));
		break;
	case SDIO_IO_DWORD:
		PLTFM_SDIO_CMD52_W8(R_AX_SDIO_INDIRECT_DATA, (u8)val);
		PLTFM_SDIO_CMD52_W8(R_AX_SDIO_INDIRECT_DATA + 1,
				    (u8)(val >> 8));
		PLTFM_SDIO_CMD52_W8(R_AX_SDIO_INDIRECT_DATA + 2,
				    (u8)(val >> 16));
		PLTFM_SDIO_CMD52_W8(R_AX_SDIO_INDIRECT_DATA + 3,
				    (u8)(val >> 24));
		PLTFM_SDIO_CMD52_W8(R_AX_SDIO_INDIRECT_CTRL,
				    (u8)(B_AX_INDIRECT_REG_W | 0x2));
		break;
	default:
		PLTFM_MSG_ERR("[ERR]invalid IO size\n");
		break;
	}

	cnt = SDIO_WAIT_CNT;
	do {
		tmp = PLTFM_SDIO_CMD52_R8(R_AX_SDIO_INDIRECT_ADDR + 3);
		cnt--;
	} while (((tmp & BIT(7)) == 0) && (cnt > 0));

	if (((tmp & BIT(7)) == 0) && cnt == 0)
		PLTFM_MSG_ERR("[ERR]sdio indirect CMD52 write\n");
}

static void w_indir_cmd53_sdio(struct mac_ax_adapter *adapter, u32 adr, u32 val,
			       enum sdio_io_size size)
{
	u8 value[12] = {0};
	u8 tmp;
	u32 cnt, i;

	switch (size) {
	case SDIO_IO_BYTE:
		*(value + 8) = (u8)B_AX_INDIRECT_REG_W;
		break;
	case SDIO_IO_WORD:
		*(value + 8) = (u8)(B_AX_INDIRECT_REG_W | 0x1);
		break;
	case SDIO_IO_DWORD:
		*(value + 8) = (u8)(B_AX_INDIRECT_REG_W | 0x2);
		break;
	default:
		PLTFM_MSG_ERR("[ERR]invalid IO size\n");
		break;
	}

	for (i = 0; i < 3; i++)
		*(value + i) = (u8)(adr >> (i << 3));
	*(value + 3) = (u8)((adr | B_AX_INDIRECT_RDY) >> 24);
	for (i = 0; i < 4; i++)
		*(value + i + 4) = (u8)(val >> (i << 3));
	PLTFM_SDIO_CMD53_WN(R_AX_SDIO_INDIRECT_ADDR, sizeof(value), value);

	cnt = SDIO_WAIT_CNT;
	do {
		tmp = PLTFM_SDIO_CMD52_R8(R_AX_SDIO_INDIRECT_ADDR + 3);
		cnt--;
	} while (((tmp & BIT(7)) == 0) && (cnt > 0));

	if (((tmp & BIT(7)) == 0) && cnt == 0)
		PLTFM_MSG_ERR("[ERR]sdio indirect CMD53 read\n");
}

static void w8_indir_sdio(struct mac_ax_adapter *adapter, u32 adr, u32 val)
{
	u8 pwr_state = pwr_state_chk_sdio(adapter);

	if (pwr_state == SDIO_PWR_OFF)
		w_indir_cmd52_sdio(adapter, adr, val, SDIO_IO_BYTE);
	else
		w_indir_cmd53_sdio(adapter, adr, val, SDIO_IO_BYTE);
}

static void w16_indir_sdio(struct mac_ax_adapter *adapter, u32 adr, u32 val)
{
	u8 pwr_state = pwr_state_chk_sdio(adapter);

	if (pwr_state == SDIO_PWR_OFF)
		w_indir_cmd52_sdio(adapter, adr, val, SDIO_IO_WORD);
	else
		w_indir_cmd53_sdio(adapter, adr, val, SDIO_IO_WORD);
}

static void w32_indir_sdio(struct mac_ax_adapter *adapter, u32 adr, u32 val)
{
	u8 pwr_state = pwr_state_chk_sdio(adapter);

	if (pwr_state == SDIO_PWR_OFF)
		w_indir_cmd52_sdio(adapter, adr, val, SDIO_IO_DWORD);
	else
		w_indir_cmd53_sdio(adapter, adr, val, SDIO_IO_DWORD);
}

static u8 pwr_state_chk_sdio(struct mac_ax_adapter *adapter)
{
	if (adapter->mac_pwr_info.pwr_seq_proc == 1 ||
	    adapter->sm.pwr == MAC_AX_PWR_OFF ||
	    adapter->mac_pwr_info.pwr_in_lps == 1 ||
	    adapter->sm.fw_rst == MAC_AX_FW_RESET_RECV ||
	    adapter->sm.fw_rst == MAC_AX_FW_RESET_RECV_DONE ||
	    adapter->sm.fw_rst == MAC_AX_FW_RESET_PROCESS) {
		adapter->sdio_info.tx_seq = 1;
		return SDIO_PWR_OFF;
	}
	return SDIO_PWR_ON;
}

static u8 reg_chk_sdio(struct mac_ax_adapter *adapter, u32 adr)
{
	u8 cv = adapter->hw_info->cv;
	u8 chip_id = adapter->hw_info->chip_id;

	if (chip_id == CHIP_WIFI6_8852A && cv <= CBV) {
		if (adr >= SDIO_LOCAL_REG_START &&
		    adr < SDIO_LOCAL_REG_END_PATCH)
			return SDIO_REG_LOCAL;
		else if (adr > SDIO_WLAN_REG_END_PATCH)
			return SDIO_REG_WLAN_PLTFM;
	} else {
		if (adr >= SDIO_LOCAL_REG_START && adr < SDIO_LOCAL_REG_END)
			return SDIO_REG_LOCAL;
		else if (adr > SDIO_WLAN_REG_END)
			return SDIO_REG_WLAN_PLTFM;
	}
	return SDIO_REG_WLAN_REG;
}

static void chk_rqd_pg_num(struct mac_ax_adapter *adapter,
			   struct mac_ax_sdio_tx_info *tx_info)
{
	u32 ple_rqd = 0;
	u16 wde_rqd = 0;
	u8 *pkt_size = tx_info->pkt_size;
	u8 *wp_offset = tx_info->wp_offset;
	u16 ple_pg_size = adapter->dle_info.ple_pg_size;
	u16 wde_pg_size = adapter->dle_info.wde_pg_size;
	u8 ple_rsvd_size = adapter->hw_info->ple_rsvd_space;
	u8 pd_size = adapter->hw_info->payload_desc_size;
	u16 size;
	u32 ple_pg_size_sh;
	u8 dma_txagg_num, i;

	ple_pg_size_sh = get_pg_size_pow(ple_pg_size);

	dma_txagg_num =
		(tx_info->dma_txagg_num == 0) ? 1 : tx_info->dma_txagg_num;
	for (i = 0; i < dma_txagg_num; i++) {
		if (tx_info->ch_dma != MAC_AX_DMA_H2C)
			size = *pkt_size + (*(pkt_size + 1) << 8) + *wp_offset +
			       ple_rsvd_size + pd_size;
		else
			size = *pkt_size + (*(pkt_size + 1) << 8) + *wp_offset;
		ple_rqd += (size >> ple_pg_size_sh) +
			   ((size & (ple_pg_size - 1)) ? 1 : 0);
		wde_rqd += wde_pg_size >> 6;
		pkt_size += 2;
		wp_offset++;
	}
	tx_info->ple_rqd_num = (u16)(ple_rqd << 1);
	tx_info->wde_rqd_num = wde_rqd;
}

static u32 chk_fs_enuf(struct mac_ax_adapter *adapter,
		       struct mac_ax_sdio_tx_info *tx_info)
{
	struct mac_ax_hfc_ch_cfg *ch_cfg = adapter->hfc_param->ch_cfg;
	struct mac_ax_hfc_ch_info *ch_info = adapter->hfc_param->ch_info;
	struct mac_ax_hfc_pub_info *pub_info = adapter->hfc_param->pub_info;
	u16 wde_min = ch_cfg[tx_info->ch_dma].min;
	u16 wde_max = ch_cfg[tx_info->ch_dma].max;
	u16 wde_use = ch_info[tx_info->ch_dma].used;
	u16 ple_thrd = adapter->hfc_param->pub_cfg->wp_thrd;
	u16 wde_aval;
	u8 i;

	wde_aval = wde_max - wde_min; //public
	for (i = 0; i < MAC_AX_DMA_CH_NUM - 1; i++) {
		wde_aval -= (ch_info[i].used < ch_cfg[i].min) ?
			    0 : (ch_info[i].used - ch_cfg[i].min);
	}
	wde_aval += (wde_min < wde_use) ? 0 : (wde_min - wde_use);

	if (wde_aval < tx_info->wde_rqd_num)
		return MACHFSWDENOTNUF;
	if (pub_info->wp_aval < tx_info->ple_rqd_num)
		return MACHFSPLENOTNUF;
	if (pub_info->wp_aval <= ple_thrd && wde_use > wde_min)
		return MACHFSPLENOTNUF;
	return MACSUCCESS;
}

static void ud_fs(struct mac_ax_adapter *adapter)
{
	u8 val[28] = {0};
	u32 fs0, fs1, fs2, fs3, fs4, fs5, fs6;
	struct mac_ax_hfc_ch_info *ch_info = adapter->hfc_param->ch_info;
	struct mac_ax_hfc_pub_info *pub_info = adapter->hfc_param->pub_info;

	PLTFM_SDIO_CMD53_RN(R_AX_SDIO_TXPG_WP, sizeof(val), val);

	fs0 = le32_to_cpu(*(u32 *)(val));
	fs1 = le32_to_cpu(*(u32 *)(val + 4));
	fs2 = le32_to_cpu(*(u32 *)(val + 8));
	fs3 = le32_to_cpu(*(u32 *)(val + 12));
	fs4 = le32_to_cpu(*(u32 *)(val + 16));
	fs5 = le32_to_cpu(*(u32 *)(val + 20));
	fs6 = le32_to_cpu(*(u32 *)(val + 24));

	ch_info[MAC_AX_DMA_H2C].aval = GET_FIELD(fs0, B_AX_SDIO_ACH12_AVAL_PG);
	pub_info->wp_aval = GET_FIELD(fs0, B_AX_SDIO_WP_AVAL_PG);
	ch_info[MAC_AX_DMA_ACH0].used = GET_FIELD(fs1, B_AX_SDIO_ACH0_USE_PG);
	ch_info[MAC_AX_DMA_ACH1].used = GET_FIELD(fs1, B_AX_SDIO_ACH1_USE_PG);
	ch_info[MAC_AX_DMA_ACH2].used = GET_FIELD(fs2, B_AX_SDIO_ACH2_USE_PG);
	ch_info[MAC_AX_DMA_ACH3].used = GET_FIELD(fs2, B_AX_SDIO_ACH3_USE_PG);
	ch_info[MAC_AX_DMA_ACH4].used = GET_FIELD(fs3, B_AX_SDIO_ACH4_USE_PG);
	ch_info[MAC_AX_DMA_ACH5].used = GET_FIELD(fs3, B_AX_SDIO_ACH5_USE_PG);
	ch_info[MAC_AX_DMA_ACH6].used = GET_FIELD(fs4, B_AX_SDIO_ACH6_USE_PG);
	ch_info[MAC_AX_DMA_ACH7].used = GET_FIELD(fs4, B_AX_SDIO_ACH7_USE_PG);
	ch_info[MAC_AX_DMA_B0MG].used = GET_FIELD(fs5, B_AX_SDIO_ACH8_USE_PG);
	ch_info[MAC_AX_DMA_B0HI].used = GET_FIELD(fs5, B_AX_SDIO_ACH9_USE_PG);
	ch_info[MAC_AX_DMA_B1MG].used = GET_FIELD(fs6, B_AX_SDIO_ACH10_USE_PG);
	ch_info[MAC_AX_DMA_B1HI].used = GET_FIELD(fs6, B_AX_SDIO_ACH11_USE_PG);
}

static u32 get_pg_size_pow(u32 size)
{
	u32 val32;

	switch (size) {
	case 64:
		val32 = 6;
		break;
	case 128:
		val32 = 7;
		break;
	case 256:
		val32 = 8;
		break;
	default:
		val32 = 8;
		break;
	}

	return val32;
}

static u32 tx_allow_data_ch(struct mac_ax_adapter *adapter,
			    struct mac_ax_sdio_tx_info *info)
{
	struct mac_ax_hfc_ch_info *ch_info = adapter->hfc_param->ch_info;
	struct mac_ax_hfc_pub_info *pub_info = adapter->hfc_param->pub_info;
	u32 cnt;
	u32 ret;

	chk_rqd_pg_num(adapter, info);

	cnt = info->chk_cnt;
	do {
		ret = chk_fs_enuf(adapter, info);
		if (ret == MACSUCCESS) {
			ch_info[info->ch_dma].used += info->wde_rqd_num;
			pub_info->wp_aval -= info->ple_rqd_num;
			break;
		}
		ud_fs(adapter);
		cnt--;
	} while (cnt);

	if (!cnt)
		return ret;

	return MACSUCCESS;
}

static u32 tx_allow_fwcmd_ch(struct mac_ax_adapter *adapter,
			     struct mac_ax_sdio_tx_info *info)
{
	struct mac_ax_hfc_ch_info *ch_info = adapter->hfc_param->ch_info;
	u32 cnt;

	chk_rqd_pg_num(adapter, info);

	cnt = info->chk_cnt;
	do {
		if (ch_info[MAC_AX_DMA_H2C].aval >= info->ple_rqd_num) {
			ch_info[MAC_AX_DMA_H2C].aval -= info->ple_rqd_num;
			break;
		}
		ud_fs(adapter);
		cnt--;
	} while (cnt);

	if (!cnt)
		return MACHFSPLENOTNUF;

	return MACSUCCESS;
}

u32 sdio_get_txagg_num(struct mac_ax_adapter *adapter, u8 band)
{
	return SDIO_DEFAULT_AGG_NUM;
}

u32 sdio_autok_counter_avg(struct mac_ax_adapter *adapter)
{
	return MACSUCCESS;
}

u32 dbcc_hci_ctrl_sdio(struct mac_ax_adapter *adapter,
		       struct mac_ax_dbcc_hci_ctrl *info)
{
	return MACSUCCESS;
}
#endif /*MAC_AX_SDIO_SUPPORT*/
