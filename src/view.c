/*
 * Copyright 2010, Christian Lamparter <chunkeey@googlemail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <stdlib.h>
#include <stdio.h>
#include <error.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "p54eeprom.h"

#include "compiler.h"

#include "dbg.c"
#include "lmac.h"
#include "p54.h"

static void dummy_desc(const struct pda_entry *head __unused, struct p54e *ee __unused)

{
	print_hex_dump_bytes("\t", head->data, (head->len - 1) * 2);
}

static void mac_desc(const struct pda_entry *head, struct p54e *ee __unused)
{
	fprintf(stdout, "\tDevice MAC: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n",
		head->data[0], head->data[1], head->data[2],
		head->data[3], head->data[4], head->data[5]);
}

#define FILL_STR(a)					\
	[(a)] = (#a)

#define FILL_STR_CONCAT(a, b)				\
	[(a## b)] = (#b)

static const char *phy_str[] = {
	[0] = "INVALID",
	FILL_STR_CONCAT(PDR_SYNTH_FRONTEND_, DUETTE3),
	FILL_STR_CONCAT(PDR_SYNTH_FRONTEND_, DUETTE2),
	FILL_STR_CONCAT(PDR_SYNTH_FRONTEND_, FRISBEE),
	FILL_STR_CONCAT(PDR_SYNTH_FRONTEND_, XBOW),
	FILL_STR_CONCAT(PDR_SYNTH_FRONTEND_, LONGBOW),
};

static const char *phy_detector[] = {
	/* huge holes :/ */
	FILL_STR_CONCAT(PDR_SYNTH_IQ_CAL_, PA_DETECTOR),
	FILL_STR_CONCAT(PDR_SYNTH_IQ_CAL_, DISABLED),
	FILL_STR_CONCAT(PDR_SYNTH_IQ_CAL_, ZIF),
};

static void intf_desc_unkn(unsigned long mask __unused)
{
}

static void intf_desc_3900(unsigned long mask)
{
	fprintf(stdout, "\t\tPHY/RF Chip : %x = %s\n", (u8)mask & PDR_SYNTH_FRONTEND_MASK,
		phy_str[mask & PDR_SYNTH_FRONTEND_MASK]);
	fprintf(stdout, "\t\tIQ Cal      : %x = %s\n", (u8)mask & PDR_SYNTH_IQ_CAL_MASK,
		phy_detector[mask & PDR_SYNTH_IQ_CAL_MASK]);
	fprintf(stdout, "\t\tFAA Switch  : %s\n", mask & PDR_SYNTH_FAA_SWITCH_ENABLED ?
		"Enabled" : "Disabled or not present");
	fprintf(stdout, "\t\tBands       : %s%s\n", mask & PDR_SYNTH_5_GHZ_DISABLED ?
		"" : "[5 GHz]", mask & PDR_SYNTH_24_GHZ_DISABLED ? "" : "[2.4 GHz]");
	fprintf(stdout, "\t\tTX Diversity: %s\n",
		(mask & PDR_SYNTH_TX_DIV_SUPPORTED) ? "Supported" : "Disabled");
	fprintf(stdout, "\t\tRX Diversity: %s\n",
		(mask & PDR_SYNTH_RX_DIV_SUPPORTED) ? "Supported" : "Disabled");
	fprintf(stdout, "\t\tASM         : %s\n",
		(mask & PDR_SYNTH_ASM_XSWON) ? "XSWON" : "XSWOFF");

}

static void intf_desc_product(unsigned long mask)
{
	fprintf(stdout, "\t\tLED Mode    : %x\n", (u8)mask);
}

struct idlist {
	const char *name;
	void (*func)(unsigned long dummy);
};

#define ADD_LOOKUP(_prefix, _name, _func)		\
	[(_prefix## _name)] = {				\
		  .name = (# _name),			\
		  .func = (_func),			\
	}

#define ADD_IF_ID_DUMMY(_name)				\
	ADD_LOOKUP(IF_ID_, _name, intf_desc_unkn)

static const struct idlist if_id_list[] = {
	ADD_LOOKUP(IF_ID_, ISL39000, intf_desc_3900),
	ADD_LOOKUP(IF_ID_, PRODUCT, intf_desc_product),

	ADD_IF_ID_DUMMY(ISL36356A), ADD_IF_ID_DUMMY(MVC),
	ADD_IF_ID_DUMMY(DEBUG), ADD_IF_ID_DUMMY(OEM),
	ADD_IF_ID_DUMMY(PCI3877), ADD_IF_ID_DUMMY(ISL37704C),
	ADD_IF_ID_DUMMY(ISL39300A), ADD_IF_ID_DUMMY(ISL37700_UAP),
	ADD_IF_ID_DUMMY(ISL39000_UAP), ADD_IF_ID_DUMMY(LMAC),
};

static const char *intf_role[] = {
	FILL_STR_CONCAT(PDR_INTERFACE_ROLE_, CLIENT),
	FILL_STR_CONCAT(PDR_INTERFACE_ROLE_, SERVER),
};

static void intf_desc(const struct pda_entry *head, struct p54e *ee __unused)
{
	const struct exp_if *intf;
	unsigned int i;

	intf = (const void *) head->data;

	for (i = 0; i < PDA_DATA_LEN(head) / sizeof(*intf); i++) {
		fprintf(stdout, "\t--- Interface %d ---\n", i);
		fprintf(stdout, "\trole         : 0x%.4x (%s)\n", le16_to_cpu(intf[i].role),
			intf_role[le16_to_cpu(intf[i].role)]);
		fprintf(stdout, "\tinterface id : 0x%.4x (%s)\n", le16_to_cpu(intf[i].if_id),
			if_id_list[le16_to_cpu(intf[i].if_id)].name);
		fprintf(stdout, "\tvariant      : 0x%.4x\n", le16_to_cpu(intf[i].variant));
		if_id_list[le16_to_cpu(intf[i].if_id)].func(le16_to_cpu(intf[i].variant));
		fprintf(stdout, "\tbottom compat: 0x%.4x\n", intf[i].btm_compat);
		fprintf(stdout, "\ttop compat   : 0x%.4x\n", intf[i].top_compat);
		fprintf(stdout, "\n");
	}
}

static void hw_desc(const struct pda_entry *head, struct p54e *ee __unused)
{
	fprintf(stdout, "\tChip Name: ISL38%.2x\n", head->data[1]);
}

#define REGDOM_GLOBAL		0x00
#define REGDOM_FCC		0x10
#define REGDOM_IC		0x20
#define REGDOM_ETSI		0x30
#define REGDOM_SPAIN		0x31
#define REGDOM_FRANCE		0x32
#define REGDOM_MKK		0x40
#define REGDOM_MKK13		0x41

static const char *regd_list[] = {
	FILL_STR_CONCAT(REGDOM_, GLOBAL),
	FILL_STR_CONCAT(REGDOM_, FCC),
	FILL_STR_CONCAT(REGDOM_, IC),
	FILL_STR_CONCAT(REGDOM_, ETSI),
	FILL_STR_CONCAT(REGDOM_, SPAIN),
	FILL_STR_CONCAT(REGDOM_, FRANCE),
	FILL_STR_CONCAT(REGDOM_, MKK),
	FILL_STR_CONCAT(REGDOM_, MKK13),
};

static void country_info(const struct pda_country *head)
{
	if (head->flags & PDR_COUNTRY_CERT_CODE)
		fprintf(stdout, "\tCountry ISO-3316: %c%c\n", head->alpha2[0], head->alpha2[1]);
	else
		fprintf(stdout, "\tRegDomain Code  : 0x%.2x %s\n", head->regdomain, regd_list[head->regdomain]);

	fprintf(stdout, "\tFlags           : 0x%.2x\n", head->flags);
	fprintf(stdout, "\t\tType          : %s\n", head->flags & PDR_COUNTRY_CERT_CODE ?
		"ISO 3316" : "RegDomain");
	fprintf(stdout, "\t\tBand          : %s\n", head->flags & PDR_COUNTRY_CERT_BAND ?
		"[5 GHz]" : "[2.4 GHz]");
	fprintf(stdout, "\t\tIn-/Out-door  : %s%s\n",
		((head->flags & PDR_COUNTRY_CERT_IODOOR) != PDR_COUNTRY_CERT_IODOOR_INDOOR ? "[Outdoor]" : ""),
		((head->flags & PDR_COUNTRY_CERT_IODOOR) != PDR_COUNTRY_CERT_IODOOR_OUTDOOR ? "[Indoor]" : ""));
}

static void cclst_desc(const struct pda_entry *head, struct p54e *ee __unused)
{
	const struct pda_country *c;
	unsigned int i;

	c = (const void *)head->data;
	for (i = 0; i < PDA_DATA_LEN(head) / sizeof(*c); i++) {
		fprintf(stdout, "\t--- Country %d ---\n", i);
		country_info(&c[i]);
		fprintf(stdout, "\n");
	}
}

static void cc_desc(const struct pda_entry *head, struct p54e *ee __unused)
{
	country_info((const struct pda_country *)head->data);
}

#define DB_STR "%d.%.2d"
#define DB_VAL(a) ((a) >> 2), (((a) & 3) * 25)

static void ant_desc(const struct pda_entry *head, struct p54e *ee __unused)
{
	const struct pda_antenna_gain *ant;
	int i;

	ant = (const void *)head->data;
	for (i = 0; i < (PDA_DATA_LEN(head) >> 1); i++) {
		fprintf(stdout, "\t--- antenna %d gain ---\n", i);
		fprintf(stdout, "\tGain 5 GHz  : " DB_STR " dBi\n",
			ant->antenna[i].gain_5GHz >> 2, (ant->antenna[i].gain_5GHz & 3) * 25);
		fprintf(stdout, "\tGain 2.4 GHz: " DB_STR " dBi\n",
			DB_VAL(ant->antenna[i].gain_2GHz));
		fprintf(stdout, "\n");
	}

}

static void database_dump(const struct pda_custom_wrapper *base)
{
	unsigned int pl, e, es, o, i;
	const char *p;

	pl = le16_to_cpu(base->len);
	e = le16_to_cpu(base->entries);
	es = le16_to_cpu(base->entry_size);
	o = le16_to_cpu(base->offset);

	fprintf(stdout, "\tDataBase Entries: %d, Entry Size: %d, Offset: %d, Payload: %d\n",
		e, es, o, pl);

	p = (const void *)base->data;
	for (i = 0; i < e; i++, p += es) {
		fprintf(stdout, "\t--- Entry %d ---\n", i);
		print_hex_dump_bytes("\t", p, es);
		fprintf(stdout, "\n");
	}
}

static void db_desc(const struct pda_entry *head, struct p54e *ee __unused)
{
	database_dump((const struct pda_custom_wrapper *)head->data);
}

static void iq_desc(const struct pda_entry *head, struct p54e *ee __unused)
{
	const struct pda_iq_autocal_entry *iq;
	unsigned int i;

	iq = (const void *)head->data;
	for (i = 0; i < PDA_DATA_LEN(head) / sizeof(*iq); i++) {
		fprintf(stdout, "\t--- IQ Auto Calibration Data for %d MHz ---\n",
			iq[i].freq);
		fprintf(stdout, "\tIQ Parameters: %6d, %6d, %6d, %6d\n",
			(int16_t)iq[i].params.iq_param[0], (int16_t)iq[i].params.iq_param[1],
			(int16_t)iq[i].params.iq_param[2], (int16_t)iq[i].params.iq_param[3]);
		fprintf(stdout, "\n");
	}
}

static const char *band_names[2] = { "2.4 GHz", "5 GHz" };

static void rssi_custom_desc(const struct pda_entry *head, struct p54e *ee __unused)
{
	const struct p54_rssi_linear_approximation *rssi_db;
	unsigned int i;

	rssi_db = (const void *)head->data;

	for (i = 0; i < PDA_DATA_LEN(head) / sizeof(*rssi_db); i++) {
		fprintf(stdout, "\t--- RSSI <-> dBm parameters for the %s band ---\n",
			band_names[i]);
		fprintf(stdout, "\tCoefficient       : %6d\n", (int16_t)rssi_db[i].mul);
		fprintf(stdout, "\tSummand           : %6d\n", (int16_t)rssi_db[i].add);
		fprintf(stdout, "\tLongbow Parameters: %6d, %6d\n",
			(int16_t)rssi_db[i].longbow_unkn,
			(int16_t)rssi_db[i].longbow_unkn2);
		fprintf(stdout, "\n");
	}
}

static void rssi_desc(const struct pda_entry *head, struct p54e *ee __unused)
{
	unsigned int offset = (head->code == cpu_to_le16(PDR_RSSI_LINEAR_APPROXIMATION_EXTENDED)) ? 2 : 0;
	unsigned int entry_size = sizeof(struct pda_rssi_cal_entry) + offset;
	unsigned int num_entries = (head->code == cpu_to_le16(PDR_RSSI_LINEAR_APPROXIMATION)) ? 1 : 2;
	unsigned int i;

	for (i = 0; i < num_entries; i++) {
		const struct pda_rssi_cal_entry *cal = (const void *)
			((unsigned long)head->data + (offset + (i * entry_size)));

		fprintf(stdout, "\t---Entry %d for band %s---\n", i, band_names[i]);
		fprintf(stdout, "\tCoefficient: %6d\n", (int16_t)cal->mul);
		fprintf(stdout, "\tSummand    : %6d\n", (int16_t)cal->add);

	}
}

static void power_limits_v0(const struct pda_entry *head)
{
	struct pda_channel_output_limit *limit;
	unsigned int len, i;

	limit = (void *)(((unsigned long)head->data) + 2);
	len = head->data[1];

	for (i = 0; i < len; i++) {
		fprintf(stdout, "\t--- Power Limit for Frequency %d MHz ---\n",
			le16_to_cpu(limit[i].freq));
		fprintf(stdout, "\tBarker: " DB_STR " dBm\n", DB_VAL(limit[i].val_barker));
		fprintf(stdout, "\tBPSK  : " DB_STR " dBm\n", DB_VAL(limit[i].val_bpsk));
		fprintf(stdout, "\tQPSK  : " DB_STR " dBm\n", DB_VAL(limit[i].val_qpsk));
		fprintf(stdout, "\t16-QAM: " DB_STR " dBm\n", DB_VAL(limit[i].val_16qam));
		fprintf(stdout, "\t64-QAM: " DB_STR " dBm\n", DB_VAL(limit[i].val_64qam));
		fprintf(stdout, "\tRate Set Mask: %.4x\n", limit[i].rate_set_mask);
		fprintf(stdout, "\n");
	}
}

static void power_limits_desc(const struct pda_entry *head, struct p54e *ee __unused)
{
	switch (head->data[0]) {
	case 0:
		power_limits_v0(head);
		break;

	default:
		fprintf(stdout, "\tUnknown output power limits database revision.\n");
		print_hex_dump_bytes("\t", head->data, PDA_DATA_LEN(head));
		break;
	}
}

static void curve_show_rev(const struct pda_pa_curve_data *header)
{
	fprintf(stdout, "\tCurve Data Revision       :  %3d\n", header->cal_method_rev);
	fprintf(stdout, "\tChannels                  :  %3d\n", header->channels);
	fprintf(stdout, "\tPoints-per-channel        :  %3d\n", header->points_per_channel);
	fprintf(stdout, "\tPA Detector Interpretation: 0x%.2x\n", header->pa_interpret);
}

static void curve_v0(const struct pda_entry *head)
{
	const struct pda_pa_curve_data *header;
	const struct pda_pa_curve_data_sample_rev0 *data;
	const uint8_t *p;
	const __le16 *f;
	unsigned int i, j;

	header = (const void *) head->data;
	p = (const void *) header->data;
	curve_show_rev(header);
	for (i = 0; i < header->channels; i++) {
		f = (const __le16 *)p;

		fprintf(stdout, "\t--- Curve Points for Frequency %4d MHz ---\n",
			le16_to_cpu(*f));

		p += sizeof(*f);
		data = (const void *) p;

		for (j = 0; j < header->points_per_channel; j++) {
			fprintf(stdout, "\t\t--- Point %d ---\n", j);
			fprintf(stdout, "\t\tRF Power           : " DB_STR " dBm\n",
				DB_VAL(data[j].rf_power));
			fprintf(stdout, "\t\tPA detector        : %d\n", data[j].pa_detector);
			fprintf(stdout, "\t\tPower Control Value: %d\n", data[j].pcv);

			fprintf(stdout, "\n");
			p += sizeof(*data);
		}

		p = (void *)ALIGN((unsigned long)p, 2);
	}
}

static void curve_v1(const struct pda_entry *head)
{
	const struct pda_pa_curve_data *header;
	const struct pda_pa_curve_data_sample_rev1 *data;
	const __le16 *f;
	const uint8_t *p;
	unsigned int i, j;

	header = (const void *) head->data;
	p = (const void *) header->data;
	curve_show_rev(header);

	for (i = 0; i < header->channels; i++) {
		f = (const __le16 *)p;

		fprintf(stdout, "\t--- Curve Points for Frequency %4d MHz ---\n",
			le16_to_cpu(*f));

		p += sizeof(*f);
		data = (const void *) p;

		for (j = 0; j < header->points_per_channel; j++) {
			fprintf(stdout, "\t\t--- Point %d ---\n", j);
			fprintf(stdout, "\t\tRF Power   : " DB_STR " dBm\n",
				DB_VAL(data[j].rf_power));
			fprintf(stdout, "\t\tPA detector: %d\n", data[j].pa_detector);
			fprintf(stdout, "\t\tBarker     : %d\n", data[j].data_barker);
			fprintf(stdout, "\t\tBPSK       : %d\n", data[j].data_bpsk);
			fprintf(stdout, "\t\tQPSK       : %d\n", data[j].data_qpsk);
			fprintf(stdout, "\t\t16-QAM     : %d\n", data[j].data_16qam);
			fprintf(stdout, "\t\t64-QAM     : %d\n", data[j].data_64qam);

			p += sizeof(*data);
		}

		p = (const void *)ALIGN((unsigned long)p, 2);
	}
}

static void curve_desc(const struct pda_entry *head, struct p54e *ee __unused)
{
	switch (head->data[0]) {
	case 0:
		curve_v0(head);
		break;
	case 1:
		curve_v1(head);
		break;
	default:
		fprintf(stdout, "\tUnknown curve database revision.\n");
		print_hex_dump_bytes("\t", head->data, PDA_DATA_LEN(head));
		break;
	}
}

static void string_desc(const struct pda_entry *head, struct p54e *ee __unused)
{
	fprintf(stdout, "\tString => \"%s\"\n", head->data);
}

#define ADD_HANDLER(_code, _func)	\
	{				\
	  .code = cpu_to_le16(_code),	\
	  .code_str = #_code,		\
	  .func = _func,		\
	}

static const struct {
	const __le16 code;
	const char *code_str;
	void (*func)(const struct pda_entry *, struct p54e *);
} known_magics[] = {
	ADD_HANDLER(PDR_MANUFACTURING_PART_NUMBER, string_desc),
	ADD_HANDLER(PDR_NIC_SERIAL_NUMBER, string_desc),
	ADD_HANDLER(PDR_MAC_ADDRESS, mac_desc),
	ADD_HANDLER(PDR_INTERFACE_LIST, intf_desc),
	ADD_HANDLER(PDR_HARDWARE_PLATFORM_COMPONENT_ID, hw_desc),
	ADD_HANDLER(PDR_COUNTRY_LIST, cclst_desc),
	ADD_HANDLER(PDR_DEFAULT_COUNTRY, cc_desc),
	ADD_HANDLER(PDR_ANTENNA_GAIN, ant_desc),
	ADD_HANDLER(PDR_PRISM_ZIF_TX_IQ_CALIBRATION, iq_desc),
	ADD_HANDLER(PDR_PRISM_PA_CAL_CURVE_DATA, curve_desc),
	ADD_HANDLER(PDR_PRISM_PA_CAL_OUTPUT_POWER_LIMITS, power_limits_desc),
	ADD_HANDLER(PDR_RSSI_LINEAR_APPROXIMATION_CUSTOM, rssi_custom_desc),
	ADD_HANDLER(PDR_RSSI_LINEAR_APPROXIMATION, rssi_desc),
	ADD_HANDLER(PDR_RSSI_LINEAR_APPROXIMATION_EXTENDED, rssi_desc),
	ADD_HANDLER(PDR_RSSI_LINEAR_APPROXIMATION_DUAL_BAND, rssi_desc),
	ADD_HANDLER(PDR_PRISM_PA_CAL_OUTPUT_POWER_LIMITS_CUSTOM, db_desc),
	ADD_HANDLER(PDR_PRISM_PA_CAL_CURVE_DATA_CUSTOM, db_desc),
	ADD_HANDLER(PDR_REGULATORY_POWER_LIMITS, dummy_desc),
	ADD_HANDLER(PDR_END, dummy_desc),
};

static void show_desc_head(const struct pda_entry *head, const char *code_str)
{
	fprintf(stdout, ">\tDescriptor size: %5d, id: %.4x (%s)\n",
		le16_to_cpu(head->len) * 2,
		le16_to_cpu(head->code), code_str);
}

static void view_help(void)
{
	fprintf(stderr, "Usage:\n");
	fprintf(stderr, "\tview EEPROM-FILE\n");

	fprintf(stderr, "\nDescription:\n");
	fprintf(stderr, "\tThis utility displays the eeprom content in "
		"human-readable form.\n");

	fprintf(stderr, "\nParameteres:\n");
	fprintf(stderr, "\t 'EEPROM-FILE'\t = eeprom file name\n");
	fprintf(stderr, "\n");
}

int main(int argc, char *args[])
{
	struct p54e *ee = NULL;
	const struct pda_entry *iter = NULL;
	unsigned int i;
	int err = 0;

	if (argc != 2) {
		err = -EINVAL;
		goto out;
	}

	ee = p54e_load_file(args[1]);
	if (IS_ERR_OR_NULL(ee)) {
		fprintf(stderr, "Failed to open file \"%s\" (%d).\n",
			args[1], (int) PTR_ERR(ee));
		goto out;
	}

	fprintf(stdout, "General EEPROM Statistics:\n");
	fprintf(stdout, "\t%d Descriptors in %d Bytes\n",
		p54e_get_descs_num(ee), p54e_get_descs_size(ee));

	fprintf(stdout, "\n");

	while ((iter = p54e_desc_next(ee, iter))) {
		for (i = 0; i < ARRAY_SIZE(known_magics); i++) {
			if (iter->code == known_magics[i].code) {
				show_desc_head(iter, known_magics[i].code_str);
				known_magics[i].func(iter, ee);
				break;
			}
		}

		if (i == ARRAY_SIZE(known_magics)) {
			show_desc_head(iter, "UNKNOWN");
			print_hex_dump_bytes("\t", iter->data, PDA_DATA_LEN(iter));
		}

		fprintf(stdout, "\n");
	}

out:
	switch (err) {
	case 0:
		break;
	case -EINVAL:
		view_help();
		break;
	default:
		break;
	}

	p54e_release(ee);
	return err ? EXIT_FAILURE : EXIT_SUCCESS;
}
