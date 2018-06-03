/*
 * Unified UUID/GUID definition
 *
 * Copyright (C) 2009, 2016 Intel Corp.
 *	Huang Ying <ying.huang@intel.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 as published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <LinuxBase.h>
#include <linux/kernel.h>
#include <linux/ctype.h>
#include <linux/errno.h>
#include <linux/export.h>
#include <linux/uuid.h>

const guid_t guid_null;
EXPORT_SYMBOL(guid_null);
const uuid_t uuid_null;
EXPORT_SYMBOL(uuid_null);

const u8 guid_index[16] = {3,2,1,0,5,4,7,6,8,9,10,11,12,13,14,15};
const u8 uuid_index[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};

/**
 * uuid_is_valid - checks if a UUID string is valid
 * @uuid:	UUID string to check
 *
 * Description:
 * It checks if the UUID string is following the format:
 *	xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
 *
 * where x is a hex digit.
 *
 * Return: true if input is valid UUID string.
 */
bool uuid_is_valid(const char *uuid)
{
	unsigned int i;

	for (i = 0; i < UUID_STRING_LEN; i++) {
		if (i == 8 || i == 13 || i == 18 || i == 23) {
			if (uuid[i] != '-')
				return false;
		} else if (!isxdigit(uuid[i])) {
			return false;
		}
	}

	return true;
}
EXPORT_SYMBOL(uuid_is_valid);

static int __uuid_parse(const char *uuid, __u8 b[16], const u8 ei[16])
{
	static const u8 si[16] = {0,2,4,6,9,11,14,16,19,21,24,26,28,30,32,34};
	unsigned int i;

	if (!uuid_is_valid(uuid))
		return -EINVAL;

	for (i = 0; i < 16; i++) {
		int hi = hex_to_bin(uuid[si[i] + 0]);
		int lo = hex_to_bin(uuid[si[i] + 1]);

		b[ei[i]] = (hi << 4) | lo;
	}

	return 0;
}

int guid_parse(const char *uuid, guid_t *u)
{
	return __uuid_parse(uuid, u->b, guid_index);
}
EXPORT_SYMBOL(guid_parse);

int uuid_parse(const char *uuid, uuid_t *u)
{
	return __uuid_parse(uuid, u->b, uuid_index);
}
EXPORT_SYMBOL(uuid_parse);
