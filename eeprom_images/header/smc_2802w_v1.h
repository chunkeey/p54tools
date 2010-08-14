/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

const unsigned char smc_2802w_v1[] = {
0x00, 0x00, 0x00, 0x00,
0x00, 0x00,
0x00, 0x00,
0x00, 0x00, 0x00, 0x00,

0x04, 0x00, 0x01, 0x01,
	0x02, 0x00, 0x00, 0x00, 0x00, 0x00,

0x0b, 0x00, 0x01, 0x10,
	0x00, 0x00,
	0x0f, 0x00,
	0x89, 0x00,
	0x01, 0x00,
	0xff, 0x00,

	0x00, 0x00,
	0x09, 0x00,
	0x03, 0x00,
	0x01, 0x00,
	0x1f, 0x00,

0x03, 0x00, 0x02, 0x10,
	0x03, 0x90, 0x00, 0x00,

0x07, 0x00, 0x07, 0x10,
	0x10, 0x00, 0x00, 0x00,
	0x30, 0x00, 0x00, 0x00,
	0x40, 0x00, 0x00, 0x00,

0x03, 0x00, 0x08, 0x10,
	0x10, 0x00, 0x00, 0x00,

0x03, 0x00, 0x00, 0x11,
	0x08, 0x08, 0x08, 0x08,

0x3a, 0x00, 0x03, 0x19,
	0x00, 0x0e,
		0x6c, 0x09, 0x54, 0x50, 0x4b, 0x41, 0x80, 0x54,
		0x71, 0x09, 0x54, 0x4f, 0x4b, 0x40, 0x80, 0x54,
		0x76, 0x09, 0x53, 0x4f, 0x4b, 0x40, 0x80, 0x54,
		0x7b, 0x09, 0x53, 0x4f, 0x4a, 0x40, 0x80, 0x53,
		0x80, 0x09, 0x53, 0x4f, 0x4a, 0x40, 0x80, 0x53,
		0x85, 0x09, 0x53, 0x4f, 0x4a, 0x3f, 0x80, 0x53,
		0x8a, 0x09, 0x53, 0x4e, 0x4a, 0x3f, 0x80, 0x53,
		0x8f, 0x09, 0x52, 0x4e, 0x49, 0x3f, 0x80, 0x53,
		0x94, 0x09, 0x52, 0x4e, 0x49, 0x3f, 0x80, 0x52,
		0x99, 0x09, 0x52, 0x4d, 0x49, 0x3e, 0x80, 0x52,
		0x9e, 0x09, 0x51, 0x4d, 0x48, 0x3e, 0x80, 0x52,
		0xa3, 0x09, 0x51, 0x4d, 0x48, 0x3e, 0x80, 0x50,
		0xa8, 0x09, 0x51, 0x4d, 0x48, 0x3e, 0x80, 0x4e,
		0xb4, 0x09, 0x51, 0x4d, 0x48, 0x3e, 0x80, 0x49,

0xb9, 0x00, 0x04, 0x19,
	0x00, 0x0e, 0x08, 0x80,
		0x6c, 0x09,
			0x5e, 0x77, 0xbc,
			0x56, 0x42, 0x99,
			0x4e, 0x18, 0x84,
			0x42, 0xea, 0x6f,
			0x36, 0xca, 0x5d,
			0x2a, 0xb4, 0x4b,
			0x1e, 0xa5, 0x39,
			0x00, 0x91, 0x0e,

		0x71, 0x09,
			0x5e, 0x78, 0xbd,
			0x56, 0x43, 0x99,
			0x4e, 0x19, 0x84,
			0x42, 0xea, 0x6f,
			0x36, 0xcb, 0x5d,
			0x2a, 0xb5, 0x4b,
			0x1e, 0xa5, 0x39,
			0x00, 0x91, 0x0e,

		 0x76, 0x09,
			0x5e, 0x7a, 0xbd,
			0x56, 0x43, 0x99,
			0x4e, 0x19, 0x85,
			0x42, 0xeb, 0x6f,
			0x36, 0xcb, 0x5d,
			0x2a, 0xb5, 0x4b,
			0x1e, 0xa6, 0x39,
			0x00, 0x91, 0x0e,

		0x7b, 0x09,
			0x5d, 0x73, 0xb8,
			0x55, 0x3f, 0x96,
			0x4d, 0x15, 0x83,
			0x41, 0xe8, 0x6d,
			0x35, 0xc9, 0x5b,
			0x29, 0xb4, 0x49,
			0x1d, 0xa5, 0x38,
			0x00, 0x92, 0x0e,

		0x80, 0x09,
			0x5d, 0x74, 0xb9,
			0x55, 0x3f, 0x97,
			0x4d, 0x16, 0x83,
			0x41, 0xe9, 0x6d,
			0x35, 0xc9, 0x5b,
			0x29, 0xb4, 0x49,
			0x1d, 0xa5, 0x38,
			0x00, 0x92, 0x0e,

		0x85, 0x09,
			0x5d, 0x76, 0xb9,
			0x55, 0x40, 0x97,
			0x4d, 0x17, 0x83,
			0x41, 0xe9, 0x6d,
			0x35, 0xca, 0x5b,
			0x29, 0xb4, 0x49,
			0x1d, 0xa5, 0x38,
			0x00, 0x92, 0x0e,

		0x8a, 0x09,
			0x5d, 0x76, 0xba,
			0x55, 0x41, 0x97,
			0x4d, 0x17, 0x83,
			0x41, 0xe9, 0x6d,
			0x35, 0xca, 0x5b,
			0x29, 0xb4, 0x49,
			0x1d, 0xa5, 0x38,
			0x00, 0x92, 0x0d,

		0x8f, 0x09,
			0x5d, 0x76, 0xba,
			0x55, 0x41, 0x97,
			0x4d, 0x17, 0x82,
			0x41, 0xea, 0x6d,
			0x35, 0xca, 0x5b,
			0x29, 0xb4, 0x49,
			0x1d, 0xa5, 0x37,
			0x00, 0x92, 0x0d,

		0x94, 0x09,
			0x5d, 0x76, 0xbb,
			0x55, 0x41, 0x97,
			0x4d, 0x18, 0x82,
			0x41, 0xea, 0x6d,
			0x35, 0xca, 0x5a,
			0x29, 0xb4, 0x49,
			0x1d, 0xa5, 0x37,
			0x00, 0x92, 0x0d,

		0x99, 0x09,
			0x5d, 0x76, 0xbb,
			0x55, 0x41, 0x97,
			0x4d, 0x18, 0x82,
			0x41, 0xea, 0x6d,
			0x35, 0xca, 0x5a,
			0x29, 0xb5, 0x48,
			0x1d, 0xa5, 0x37,
			0x00, 0x92, 0x0c,

		0x9e, 0x09,
			0x5d, 0x77, 0xbc,
			0x55, 0x42, 0x97,
			0x4d, 0x18, 0x82,
			0x41, 0xea, 0x6c,
			0x35, 0xcb, 0x5a,
			0x29, 0xb5, 0x48,
			0x1d, 0xa5, 0x37,
			0x00, 0x91, 0x0c,

		0xa3, 0x09,
			0x5d, 0x75, 0xbc,
			0x55, 0x41, 0x97,
			0x4d, 0x18, 0x82,
			0x41, 0xea, 0x6c,
			0x35, 0xcb, 0x5a,
			0x29, 0xb5, 0x48,
			0x1d, 0xa5, 0x36,
			0x00, 0x91, 0x0c,

		0xa8, 0x09,
			0x5e, 0x7a, 0xc4,
			0x56, 0x45, 0x9a,
			0x4e, 0x1c, 0x84,
			0x42, 0xee, 0x6d,
			0x36, 0xcd, 0x5b,
			0x2a, 0xb6, 0x49,
			0x1e, 0xa6, 0x37,
			0x00, 0x91, 0x0b,

		0xb4, 0x09,
			0x5f, 0x79, 0xcd,
			0x57, 0x46, 0x9e,
			0x4f, 0x1e, 0x85,
			0x43, 0xf0, 0x6e,
			0x37, 0xcf, 0x5b,
			0x2b, 0xb8, 0x4a,
			0x1f, 0xa7, 0x38,
			0x00, 0x8f, 0x0a,

0x05, 0x00, 0x05, 0x19,
	0xff, 0x7f, 0xff, 0x7f, 0x80, 0x00, 0x6b, 0xfe,

0x47, 0x00, 0x06, 0x19,
	0x6c, 0x09, 0xfa, 0x03, 0x0a, 0x00, 0x02, 0x00, 0x0a, 0x01,
	0x71, 0x09, 0xfa, 0x03, 0x09, 0x00, 0x03, 0x00, 0x0a, 0x01,
	0x76, 0x09, 0xf8, 0x03, 0x08, 0x00, 0x04, 0x00, 0x0d, 0x01,
	0x7b, 0x09, 0xf8, 0x03, 0x09, 0x00, 0x03, 0x00, 0x0b, 0x01,
	0x80, 0x09, 0xf8, 0x03, 0x08, 0x00, 0x04, 0x00, 0x0c, 0x01,
	0x85, 0x09, 0xfa, 0x03, 0x09, 0x00, 0x02, 0x00, 0x0b, 0x01,
	0x8a, 0x09, 0xfb, 0x03, 0x0a, 0x00, 0x01, 0x00, 0x09, 0x01,
	0x8f, 0x09, 0xfb, 0x03, 0x0a, 0x00, 0x01, 0x00, 0x09, 0x01,
	0x94, 0x09, 0xfa, 0x03, 0x09, 0x00, 0x01, 0x00, 0x0a, 0x01,
	0x99, 0x09, 0xf9, 0x03, 0x08, 0x00, 0x02, 0x00, 0x0c, 0x01,
	0x9e, 0x09, 0xfa, 0x03, 0x08, 0x00, 0x02, 0x00, 0x0b, 0x01,
	0xa3, 0x09, 0xfd, 0x03, 0x0b, 0x00, 0x00, 0x00, 0x08, 0x01,
	0xa8, 0x09, 0xfc, 0x03, 0x09, 0x00, 0x00, 0x00, 0x0a, 0x01,
	0xb4, 0x09, 0xff, 0x03, 0x05, 0x00, 0x00, 0x00, 0x0a, 0x01,

0x02, 0x00, 0x00, 0x00,
	0xc1, 0xb1,
};
