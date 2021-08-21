#pragma once

#include <osmocom/core/endian.h>

/* Section 9.4.1.2: GSM Message Format */
struct gsm23041_msg_param_gsm {
	uint16_t serial_nr;
	uint16_t message_id;
	uint8_t dcs;
	struct {
#if OSMO_IS_LITTLE_ENDIAN
		uint8_t num_pages:4,
			page_nr:4;
#elif OSMO_IS_BIG_ENDIAN
/* auto-generated from the little endian part above (libosmocore/contrib/struct_endianess.py) */
		uint8_t page_nr:4, num_pages:4;
#endif
	} page_param;
	uint8_t content[0];
} __attribute__ ((packed));

/* Section 9.4.1.2.2 Message Identifier */
enum {
	/* 0 - 999: Allocated by GSM AD.26 */
	CBS_MSGID_LCS_EOTD_ASSIST	= 1000,
	CBS_MSGID_LCS_DGPS_CORRECTION	= 1001,
	CBS_MSGID_LCS_GPS_EPHEM_CLOCK	= 1002,
	CBS_MSGID_LCS_GPS_ALMANAC_OTHER	= 1003,
	/* 1004 - 4095: RFU */
	/* 4096 - 4223: clear text SIM data download */
	/* 4224 - 4351: secured SIM data download */
	CBS_MSGID_ETWS_EARTHQUAKE	= 4352,
	CBS_MSGID_ETWS_TSUNAMI		= 4353,
	CBS_MSGID_ETWS_EARTHQUAKE_TSUNAMI = 4354,
	CBS_MSGID_ETWS_TEST		= 4355,
	CBS_MSGID_ETWS_OTHER		= 4356,
	/* 4357 - 4359: ETWS RFU */
	/* 4360 - 4369: RFU */
	CBS_MSGID_CMAS_PRESIDENTIAL	= 4370,
	CBS_MSGID_CMAS_EXTREME_IMMEDIATE_OBSERVED	= 4371,
	CBS_MSGID_CMAS_EXTREME_IMMEDIATE_LIKELY		= 4372,
	CBS_MSGID_CMAS_EXTREME_EXPECTED_OBSERVED	= 4373,
	CBS_MSGID_CMAS_EXTREME_EXPECTED_LIKELY		= 4374,
	CBS_MSGID_CMAS_SEVERE_IMMEDIATE_OBSERVED	= 4375,
	CBS_MSGID_CMAS_SEVERE_IMMEDIATE_LIKELY		= 4376,
	CBS_MSGID_CMAS_SEVERE_EXPECTED_OBSERVED		= 4377,
	CBS_MSGID_CMAS_SEVERE_EXPECTED_LIKELY		= 4378,
	CBS_MSGID_CMAS_AMBER				= 4379,
	CBS_MSGID_CMAS_MONTHLY_TEST			= 4380,
	CBS_MSGID_CMAS_EXERCISE				= 4381,
	CBS_MSGID_CMAS_OPERATOR_DEFINED			= 4382,
	CBS_MSGID_CMAS_PRESIDENTIAL_ADDL		= 4383,
	CBS_MSGID_CMAS_EXTREME_IMMEDIATE_OBSERVED_ADDL	= 4384,
	CBS_MSGID_CMAS_EXTREME_IMMEDIATE_LIKELY_ADDL	= 4385,
	CBS_MSGID_CMAS_EXTREME_EXPECTED_OBSERVED_ADDL	= 4386,
	CBS_MSGID_CMAS_EXTREME_EXPECTED_LIKELY_ADDL	= 4387,
	CBS_MSGID_CMAS_SEVERE_IMMEDIATE_OBSERVED_ADDL	= 4388,
	CBS_MSGID_CMAS_SEVERE_IMMEDIATE_LIKELY_ADDL	= 4389,
	CBS_MSGID_CMAS_SEVERE_EXPECTED_OBSERVED_ADDL	= 4390,
	CBS_MSGID_CMAS_SEVERE_EXPECTED_LIKELY_ADDL	= 4391,
	CBS_MSGID_CMAS_AMBER_ADDL			= 4392,
	CBS_MSGID_CMAS_MONTHLY_TEST_ADDL		= 4393,
	CBS_MSGID_CMAS_EXERCISE_ADDL			= 4394,
	CBS_MSGID_CMAS_OPERATOR_DEFINED_ADDL		= 4395,
	/* 4396 - 4399: RFU CMAS / EU-Alert */
	/* 4400 - 6399: RFU PWS */
	CBS_MSGID_EU_INFO_LOCAL_LANG			= 6400,
	/* 6491 - 40959: RFU */
	/* 40960 - 45055: PLMN operator specific range */
	/* 45056 - 61439: PLMN operator specific range RFU */
	/* 61440 - 65534: PLMN operator specific range */
	CBS_MSGID_RESERVED				= 65535
};

/* Section 9.4.1.3.2 ETWS Primary Notification Message Parameter */
struct gsm23041_etws_primary_gsm {
	uint16_t serial_nr;
	uint16_t message_id;
	uint16_t warning_type;
	uint8_t warning_sec_info[50];
} __attribute__ ((packed));


/* Section 9.4.2.2 UMTS Message Parameter */
struct gsm23041_msg_param_umts {
	uint8_t msg_type;	/* as per TS 25.324 */
	uint16_t message_id;
	uint16_t serial_nr;
	uint8_t dcs;
	uint8_t content[0];
} __attribute__ ((packed));
