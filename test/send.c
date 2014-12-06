
#include <check.h>
#include "test/print_safe_buffer.h"

/*
 * http://check.sourceforge.net/doc/check_html/check_3.html
 *
 * http://entrenchant.blogspot.com/2010/08/unit-testing-in-c.html
 */

#ifndef countof
# define countof(x) (sizeof(x)/sizeof(x[0]))
#endif

typedef union {
	struct sockaddr_in6 in6;
	struct sockaddr_in in;
} sockaddr;

int getaddrs(struct ifaddrs **ifap)
{
	static struct ifaddrs ifa[8*4*1024];
	static sockaddr addrs[countof(ifa)];
	static struct sockaddr_in6 mask;
	static char * names[4*1024] = {0};

	memset(addrs, 0, sizeof(addrs));
	memset(ifa, 0, sizeof(ifa));

	inet_pton(AF_INET6, "ffff:ffff:ffff:ffff::", &mask.sin6_addr);

	if (0 == names[0]) {
		for (int i = 0; i < countof(names); ++i) {
			char buffer[IFNAMSIZ];
			snprintf(buffer, sizeof(buffer), "fake%d", i);
			names[i] = strdup(buffer);
		}
	}

	for (int i = 0; i < countof(ifa); ++i) {
		int j = i / 8;
		switch (i%8) {
		case 7:
			addrs[i].in.sin_addr.s_addr = (j & 0xffffff);
			addrs[i].in.sin_addr.s_addr |= 0x0c000000;
			addrs[i].in.sin_addr.s_addr = htonl(addrs[i].in.sin_addr.s_addr);
			addrs[i].in.sin_family = AF_INET;
			break;

		case 6:
			addrs[i].in.sin_addr.s_addr = (j & 0xffffff);
			addrs[i].in.sin_addr.s_addr |= 0x0b000000;
			addrs[i].in.sin_addr.s_addr = htonl(addrs[i].in.sin_addr.s_addr);
			addrs[i].in.sin_family = AF_INET;
			break;

		case 5:
			addrs[i].in.sin_addr.s_addr = (j & 0xffffff);
			addrs[i].in.sin_addr.s_addr |= 0x0a000000;
			addrs[i].in.sin_addr.s_addr = htonl(addrs[i].in.sin_addr.s_addr);
			addrs[i].in.sin_family = AF_INET;
			break;
		case 4:
			inet_pton(AF_INET6, "fe80:f00d::1", &addrs[i].in6.sin6_addr);
			addrs[i].in6.sin6_addr.s6_addr[7] += j & 0xff;
			addrs[i].in6.sin6_addr.s6_addr[6] += (j>>8) & 0xff;
			addrs[i].in6.sin6_family = AF_INET6;
			break;

		case 3:
			inet_pton(AF_INET6, "fe80:cafe::1", &addrs[i].in6.sin6_addr);
			addrs[i].in6.sin6_addr.s6_addr[7] += j & 0xff;
			addrs[i].in6.sin6_addr.s6_addr[6] += (j>>8) & 0xff;
			addrs[i].in6.sin6_family = AF_INET6;
			break;

		case 2:
			inet_pton(AF_INET6, "2001:feed::1", &addrs[i].in6.sin6_addr);
			addrs[i].in6.sin6_addr.s6_addr[7] += j & 0xff;
			addrs[i].in6.sin6_addr.s6_addr[6] += (j>>8) & 0xff;
			addrs[i].in6.sin6_family = AF_INET6;
			break;

		case 1:
			inet_pton(AF_INET6, "2001:cafe::1", &addrs[i].in6.sin6_addr);
			addrs[i].in6.sin6_addr.s6_addr[7] += j & 0xff;
			addrs[i].in6.sin6_addr.s6_addr[6] += (j>>8) & 0xff;
			addrs[i].in6.sin6_family = AF_INET6;
			break;

		default:
			inet_pton(AF_INET6, "2001:f00d::1", &addrs[i].in6.sin6_addr);
			addrs[i].in6.sin6_addr.s6_addr[7] += j & 0xff;
			addrs[i].in6.sin6_addr.s6_addr[6] += (j>>8) & 0xff;
			addrs[i].in6.sin6_family = AF_INET6;
			break;
		}
	}

	for (int i = 0; i < countof(ifa); ++i) {
		int name_index = countof(names) * (i / (double)countof(ifa));
		ifa[i].ifa_name = names[name_index % countof(names)];
		ifa[i].ifa_addr = (struct sockaddr*)&addrs[i];
		ifa[i].ifa_netmask = (struct sockaddr*)&mask;
		ifa[i].ifa_next = &ifa[i] + 1;
	}
	ifa[countof(ifa)-1].ifa_next = 0;
#if 0
	for (struct ifaddrs * ifa2 = ifa; ifa2; ifa2 = ifa2->ifa_next) {
		char dst[INET6_ADDRSTRLEN];
		if (ifa2->ifa_addr->sa_family == AF_INET6) {
			char mask[INET6_ADDRSTRLEN];
			struct sockaddr_in6 *s6 = (struct sockaddr_in6 *)ifa2->ifa_addr;
			inet_ntop(AF_INET6, &s6->sin6_addr, dst, sizeof(dst));
			printf("%s 6 %s 64\n", ifa2->ifa_name, dst);
		}
		else if (ifa2->ifa_addr->sa_family == AF_INET) {
			struct sockaddr_in *s = (struct sockaddr_in *)ifa2->ifa_addr;
			inet_ntop(AF_INET, &s->sin_addr, dst, sizeof(dst));
			printf("%s 4 %s\n", ifa2->ifa_name, dst);
		}
	}
#endif
	*ifap = ifa;
	return 0;
}

void freeaddrs(struct ifaddrs *ifa)
{
}

START_TEST (test_decrement_lifetime)
{
	uint32_t lifetime = 10;
	decrement_lifetime(7, &lifetime);
	ck_assert_int_eq(lifetime, 3);
	decrement_lifetime(7, &lifetime);
	ck_assert_int_eq(lifetime, 0);
}
END_TEST

static struct Interface * iface = 0;

static void iface_setup(void)
{
	ck_assert_ptr_eq(0, iface);
	iface = readin_config("test/test1.conf");
	ck_assert_ptr_ne(0, iface);
}

static void iface_teardown(void)
{
	ck_assert_ptr_ne(0, iface);
	free_ifaces(iface);
	iface = 0;
}

START_TEST (test_add_ra_header)
{

	struct safe_buffer sb = SAFE_BUFFER_INIT;
	add_ra_header(&sb, &iface->ra_header_info, iface->state_info.cease_adv);

#ifdef PRINT_SAFE_BUFFER
	print_safe_buffer(&sb);
#else
	unsigned char expected[] = {
		0x86, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	};

	ck_assert_int_eq(sizeof(expected), sb.used);
	ck_assert_int_eq(0, memcmp(expected, sb.buffer, sb.used));
#endif

	safe_buffer_free(&sb);
}
END_TEST

START_TEST (test_add_prefix)
{
	ck_assert_ptr_ne(0, iface);

	struct safe_buffer sb = SAFE_BUFFER_INIT;
	add_prefix(&sb, iface->AdvPrefixList, iface->state_info.cease_adv);

#ifdef PRINT_SAFE_BUFFER
	print_safe_buffer(&sb);
#else
	unsigned char expected[] = {
		0x03, 0x04, 0x40, 0xe0, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
		0xfe, 0x80, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x03, 0x04, 0x30, 0x80, 0x00, 0x00, 0x27, 0x10,
		0x00, 0x00, 0x03, 0xe8, 0x00, 0x00, 0x00, 0x00,
		0xfe, 0x80, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x03, 0x04, 0x40, 0xc0, 0x00, 0x01, 0x51, 0x80,
		0x00, 0x00, 0x38, 0x40, 0x00, 0x00, 0x00, 0x00,
		0xfe, 0x80, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	};

	ck_assert_int_eq(sizeof(expected), sb.used);
	ck_assert_int_eq(0, memcmp(expected, sb.buffer, sb.used));
#endif

	safe_buffer_free(&sb);
}
END_TEST

START_TEST (test_add_prefix_auto_zero)
{
	static struct Interface * iface_auto = 0;
	iface_auto = readin_config("test/test_auto_zero.conf");
	ck_assert_ptr_ne(0, iface_auto);

	set_debuglevel(5);
	log_open(L_STDERR, "test", 0, 0);

	struct safe_buffer sb = SAFE_BUFFER_INIT;
	build_ra(&sb, iface_auto);

#define PRINT_SAFE_BUFFER
#ifdef PRINT_SAFE_BUFFER
	print_safe_buffer(&sb);
#else
	unsigned char expected2[] = {
	};

	ck_assert_int_eq(sizeof(expected), sb.used);
	ck_assert_int_eq(0, memcmp(expected, sb.buffer, sb.used));
#endif

	safe_buffer_free(&sb);
	free_ifaces(iface_auto);
}
END_TEST


START_TEST (test_add_prefix_auto_base6)
{
	static struct Interface * iface_auto = 0;
	iface_auto = readin_config("test/test_auto_base6.conf");
	ck_assert_ptr_ne(0, iface_auto);

	set_debuglevel(5);
	log_open(L_STDERR, "test", 0, 0);

	struct safe_buffer sb = SAFE_BUFFER_INIT;
	build_ra(&sb, iface_auto);

#define PRINT_SAFE_BUFFER
#ifdef PRINT_SAFE_BUFFER
	print_safe_buffer(&sb);
#else
	unsigned char expected2[] = {
	};

	ck_assert_int_eq(sizeof(expected), sb.used);
	ck_assert_int_eq(0, memcmp(expected, sb.buffer, sb.used));
#endif

	safe_buffer_free(&sb);
	free_ifaces(iface_auto);
}
END_TEST


START_TEST (test_add_prefix_auto_base6to4)
{
	static struct Interface * iface_auto = 0;
	iface_auto = readin_config("test/test_auto_base6to4.conf");
	ck_assert_ptr_ne(0, iface_auto);

	set_debuglevel(5);
	log_open(L_STDERR, "test", 0, 0);

	struct safe_buffer sb = SAFE_BUFFER_INIT;
	build_ra(&sb, iface_auto);

#define PRINT_SAFE_BUFFER
#ifdef PRINT_SAFE_BUFFER
	print_safe_buffer(&sb);
#else
	unsigned char expected2[] = {
	};

	ck_assert_int_eq(sizeof(expected), sb.used);
	ck_assert_int_eq(0, memcmp(expected, sb.buffer, sb.used));
#endif

	safe_buffer_free(&sb);
	free_ifaces(iface_auto);
}
END_TEST


START_TEST (test_add_prefix_auto)
{
	static struct Interface * iface_auto = 0;
	iface_auto = readin_config("test/test_auto.conf");
	ck_assert_ptr_ne(0, iface_auto);

	set_debuglevel(5);
	log_open(L_STDERR, "test", 0, 0);

	struct safe_buffer sb = SAFE_BUFFER_INIT;
	build_ra(&sb, iface_auto);

#define PRINT_SAFE_BUFFER
#ifdef PRINT_SAFE_BUFFER
	print_safe_buffer(&sb);
#else
	unsigned char expected2[] = {
	};

	ck_assert_int_eq(sizeof(expected), sb.used);
	ck_assert_int_eq(0, memcmp(expected, sb.buffer, sb.used));
#endif

	safe_buffer_free(&sb);

	sb = SAFE_BUFFER_INIT;
	build_ra(&sb, iface_auto->next);

#ifdef PRINT_SAFE_BUFFER
	print_safe_buffer(&sb);
#else
	ck_assert_int_eq(sizeof(expected2), sb.used);
	ck_assert_int_eq(0, memcmp(expected2, sb.buffer, sb.used));
#endif

	safe_buffer_free(&sb);
	free_ifaces(iface_auto);
}
END_TEST

START_TEST (test_add_route)
{
	ck_assert_ptr_ne(0, iface);

	struct safe_buffer sb = SAFE_BUFFER_INIT;
	add_route(&sb, iface->AdvRouteList, iface->state_info.cease_adv);

#ifdef PRINT_SAFE_BUFFER
	print_safe_buffer(&sb);
#else
	unsigned char expected[] = {
		0x18, 0x03, 0x30, 0x18, 0x00, 0x00, 0x27, 0x10,
		0xfe, 0x80, 0x00, 0x0f, 0x00, 0x01, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x18, 0x03, 0x28, 0x08, 0xff, 0xff, 0xff, 0xff,
		0xfe, 0x80, 0x00, 0x0f, 0x00, 0x02, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x18, 0x03, 0x20, 0x00, 0x00, 0x00, 0x0b, 0xb8,
		0xfe, 0x80, 0x00, 0x0f, 0x00, 0x02, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	};

	ck_assert_int_eq(sizeof(expected), sb.used);
	ck_assert_int_eq(0, memcmp(expected, sb.buffer, sb.used));
#endif
	safe_buffer_free(&sb);
}
END_TEST


START_TEST (test_add_rdnss)
{
	ck_assert_ptr_ne(0, iface);

	struct safe_buffer sb = SAFE_BUFFER_INIT;
	add_rdnss(&sb, iface->AdvRDNSSList, iface->state_info.cease_adv);

#ifdef PRINT_SAFE_BUFFER
	print_safe_buffer(&sb);
#else
	unsigned char expected[] = {
		0x19, 0x07, 0x00, 0x00, 0x00, 0x00, 0x04, 0xd2,
		0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
		0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
		0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03,
	};

	ck_assert_int_eq(sizeof(expected), sb.used);
	ck_assert_int_eq(0, memcmp(expected, sb.buffer, sb.used));
#endif

	safe_buffer_free(&sb);
}
END_TEST


START_TEST (test_add_rdnss2)
{
	static struct Interface * iface = 0;
	iface = readin_config("test/test_rdnss.conf");
	ck_assert_ptr_ne(0, iface);

	struct safe_buffer sb = SAFE_BUFFER_INIT;
	add_rdnss(&sb, iface->AdvRDNSSList, iface->state_info.cease_adv);
	free_ifaces(iface);

#ifdef PRINT_SAFE_BUFFER
	print_safe_buffer(&sb);
#else
	unsigned char expected[] = {
		0x19, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2d,
		0x12, 0x34, 0x04, 0x23, 0xfe, 0xfe, 0x04, 0x93,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
	};

	ck_assert_int_eq(sizeof(expected), sb.used);
	ck_assert_int_eq(0, memcmp(expected, sb.buffer, sb.used));
#endif

	safe_buffer_free(&sb);
}
END_TEST


START_TEST (test_add_dnssl)
{
	ck_assert_ptr_ne(0, iface);

	struct safe_buffer sb = SAFE_BUFFER_INIT;
	add_dnssl(&sb, iface->AdvDNSSLList, iface->state_info.cease_adv);

#ifdef PRINT_SAFE_BUFFER
	print_safe_buffer(&sb);
#else
	unsigned char expected[] = {
		0x1f, 0x09, 0x00, 0x00, 0x00, 0x00, 0x03, 0xe8,
		0x06, 0x6f, 0x66, 0x66, 0x69, 0x63, 0x65, 0x06,
		0x62, 0x72, 0x61, 0x6e, 0x63, 0x68, 0x07, 0x65,
		0x78, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x03, 0x63,
		0x6f, 0x6d, 0x00, 0x06, 0x62, 0x72, 0x61, 0x6e,
		0x63, 0x68, 0x07, 0x65, 0x78, 0x61, 0x6d, 0x70,
		0x6c, 0x65, 0x03, 0x63, 0x6f, 0x6d, 0x00, 0x07,
		0x65, 0x78, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x03,
		0x63, 0x6f, 0x6d, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x1f, 0x09, 0x00, 0x00, 0x00, 0x00, 0x04, 0x4b,
		0x06, 0x6f, 0x66, 0x66, 0x69, 0x63, 0x65, 0x06,
		0x62, 0x72, 0x61, 0x6e, 0x63, 0x68, 0x07, 0x65,
		0x78, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x03, 0x6e,
		0x65, 0x74, 0x00, 0x06, 0x62, 0x72, 0x61, 0x6e,
		0x63, 0x68, 0x07, 0x65, 0x78, 0x61, 0x6d, 0x70,
		0x6c, 0x65, 0x03, 0x6e, 0x65, 0x74, 0x00, 0x07,
		0x65, 0x78, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x03,
		0x6e, 0x65, 0x74, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x1f, 0x07, 0x00, 0x00, 0x00, 0x00, 0x04, 0x4c,
		0x06, 0x6f, 0x66, 0x66, 0x69, 0x63, 0x65, 0x06,
		0x62, 0x72, 0x61, 0x6e, 0x63, 0x68, 0x07, 0x65,
		0x78, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x00, 0x06,
		0x62, 0x72, 0x61, 0x6e, 0x63, 0x68, 0x07, 0x65,
		0x78, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x00, 0x07,
		0x65, 0x78, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x00,
	};

	ck_assert_int_eq(sizeof(expected), sb.used);
	ck_assert_int_eq(0, memcmp(expected, sb.buffer, sb.used));
#endif

	safe_buffer_free(&sb);
}
END_TEST


START_TEST (test_add_mtu)
{
	ck_assert_ptr_ne(0, iface);

	struct safe_buffer sb = SAFE_BUFFER_INIT;
	add_mtu(&sb, iface->AdvLinkMTU);

#ifdef PRINT_SAFE_BUFFER
	print_safe_buffer(&sb);
#else
	unsigned char expected[] = {
		0x05, 0x01, 0x00, 0x00, 0x00, 0x00, 0x04, 0xd2,
	};

	ck_assert_int_eq(sb.used, sizeof(expected));
	ck_assert_int_eq(0, memcmp(expected, sb.buffer, sizeof(expected)));
#endif

	safe_buffer_free(&sb);
}
END_TEST

START_TEST (test_add_sllao)
{
	struct sllao sllao48 = {
		{1, 2, 3, 4, 5, 6, 7, 8},
		48,
		64,
		1500,
	};
	
	struct safe_buffer sb = SAFE_BUFFER_INIT;
	add_sllao(&sb, &sllao48);

#ifdef PRINT_SAFE_BUFFER
	print_safe_buffer(&sb);
#else
	unsigned char expected48[] = {
		0x01, 0x01, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
	};

	ck_assert_int_eq(sizeof(expected48), sb.used);
	ck_assert_int_eq(0, memcmp(sb.buffer, expected48, sizeof(expected48)));
#endif

	safe_buffer_free(&sb);

	struct sllao sllao64 = {
		{1, 2, 3, 4, 5, 6, 7, 8},
		64,
		64,
		1500,
	};
	
	sb = SAFE_BUFFER_INIT;
	add_sllao(&sb, &sllao64);

#ifdef PRINT_SAFE_BUFFER
	print_safe_buffer(&sb);
#else
	unsigned char expected64[] = {
		0x01, 0x02, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
		0x07, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	};

	ck_assert_int_eq(sizeof(expected64), sb.used);
	ck_assert_int_eq(0, memcmp(sb.buffer, expected64, sizeof(expected64)));
#endif

	safe_buffer_free(&sb);
}
END_TEST

START_TEST (test_add_lowpanco)
{
	ck_assert_ptr_ne(0, iface);

	struct safe_buffer sb = SAFE_BUFFER_INIT;
	add_lowpanco(&sb, iface->AdvLowpanCoList);

#ifdef PRINT_SAFE_BUFFER
	print_safe_buffer(&sb);
#else
	unsigned char expected[] = {
		0x22, 0x03, 0x32, 0x48, 0x00, 0x00, 0xe8, 0x03,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	};

	ck_assert_int_eq(sb.used, sizeof(expected));
	ck_assert_int_eq(0, memcmp(expected, sb.buffer, sizeof(expected)));
#endif

	safe_buffer_free(&sb);
}
END_TEST


START_TEST (test_add_abro)
{
	ck_assert_ptr_ne(0, iface);

	struct safe_buffer sb = SAFE_BUFFER_INIT;
	add_abro(&sb, iface->AdvAbroList);

#ifdef PRINT_SAFE_BUFFER
	print_safe_buffer(&sb);
#else
	unsigned char expected[] = {
		0x23, 0x03, 0x0a, 0x00, 0x02, 0x00, 0x02, 0x00,
		0xfe, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0xa2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
	};

	ck_assert_int_eq(sb.used, sizeof(expected));
	ck_assert_int_eq(0, memcmp(expected, sb.buffer, sizeof(expected)));
#endif

	safe_buffer_free(&sb);
}
END_TEST


Suite * send_suite(void)
{
	set_debuglevel(5);
	log_open(L_STDERR, "test", 0, 0);

	TCase * tc_update = tcase_create("update");
	tcase_add_test(tc_update, test_decrement_lifetime);

	TCase * tc_auto = tcase_create("auto");
	tcase_add_test(tc_auto, test_add_prefix_auto_zero);
	tcase_add_test(tc_auto, test_add_prefix_auto_base6);
	tcase_add_test(tc_auto, test_add_prefix_auto_base6to4);
	tcase_add_test(tc_auto, test_add_prefix_auto);

	TCase * tc_build = tcase_create("build");
	tcase_add_unchecked_fixture(tc_build, iface_setup, iface_teardown);
	tcase_add_test(tc_build, test_add_ra_header);
	tcase_add_test(tc_build, test_add_prefix);
	tcase_add_test(tc_build, test_add_route);
	tcase_add_test(tc_build, test_add_rdnss);
	tcase_add_test(tc_build, test_add_rdnss2);
	tcase_add_test(tc_build, test_add_dnssl);
	tcase_add_test(tc_build, test_add_mtu);
	tcase_add_test(tc_build, test_add_sllao);
	tcase_add_test(tc_build, test_add_lowpanco);
	tcase_add_test(tc_build, test_add_abro);

	Suite *s = suite_create("send");
	suite_add_tcase(s, tc_update);
	suite_add_tcase(s, tc_build);
	suite_add_tcase(s, tc_auto);

	return s;	
}

