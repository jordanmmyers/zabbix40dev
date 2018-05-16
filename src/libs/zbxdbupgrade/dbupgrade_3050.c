/*
** Zabbix
** Copyright (C) 2001-2018 Zabbix SIA
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**/

#include "common.h"
#include "db.h"
#include "dbupgrade.h"
#include "zbxtasks.h"

extern unsigned char	program_type;

/*
 * 4.0 development database patches
 */

#ifndef HAVE_SQLITE3

extern unsigned char program_type;

static int	DBpatch_3050000(void)
{
	const ZBX_FIELD	field = {"proxy_address", "", NULL, NULL, 255, ZBX_TYPE_CHAR, ZBX_NOTNULL, 0};

	return DBadd_field("hosts", &field);
}

static int	DBpatch_3050001(void)
{
	DB_RESULT	result;
	DB_ROW		row;
	int		ret = FAIL;

	/* type : 'problem' - WIDGET_PROBLEMS */
	result = DBselect(
			"select wf.widgetid,wf.name"
			" from widget w,widget_field wf"
			" where w.widgetid=wf.widgetid"
				" and w.type='problems'"
				" and wf.name like 'tags.tag.%%'");

	while (NULL != (row = DBfetch(result)))
	{
		const char	*p;
		int		index;
		zbx_uint64_t	widget_fieldid;

		if (NULL == (p = strrchr(row[1], '.')) || SUCCEED != is_uint31(p + 1, &index))
			continue;

		widget_fieldid = DBget_maxid_num("widget_field", 1);

		/* type      : 0 - ZBX_WIDGET_FIELD_TYPE_INT32 */
		/* value_int : 0 - TAG_OPERATOR_LIKE */
		if (ZBX_DB_OK > DBexecute(
				"insert into widget_field (widget_fieldid,widgetid,type,name,value_int)"
				"values (" ZBX_FS_UI64 ",%s,0,'tags.operator.%d',0)", widget_fieldid, row[0], index)) {
			goto clean;
		}
	}

	ret = SUCCEED;
clean:
	DBfree_result(result);

	return ret;
}

static int	DBpatch_3050004(void)
{
	const ZBX_FIELD	field = {"name", "", NULL, NULL, 2048, ZBX_TYPE_CHAR, ZBX_NOTNULL, 0};

	if (SUCCEED != DBadd_field("events", &field))
		return FAIL;

	return SUCCEED;
}

static int	DBpatch_3050005(void)
{
	const ZBX_FIELD	field = {"name", "", NULL, NULL, 2048, ZBX_TYPE_CHAR, ZBX_NOTNULL, 0};

	if (SUCCEED != DBadd_field("problem", &field))
		return FAIL;

	return SUCCEED;
}

#define	ZBX_DEFAULT_INTERNAL_TRIGGER_EVENT_NAME	"Cannot calculate trigger expression."
#define	ZBX_DEFAULT_INTERNAL_ITEM_EVENT_NAME	"Cannot obtain item value."

static int	DBpatch_3050008(void)
{
	int		res;
	char		*trdefault = (char *)ZBX_DEFAULT_INTERNAL_TRIGGER_EVENT_NAME;

	if (0 == (program_type & ZBX_PROGRAM_TYPE_SERVER))
		return SUCCEED;

	res = DBexecute("update events set name='%s' where source=%d and object=%d and value=%d", trdefault,
			EVENT_SOURCE_INTERNAL, EVENT_OBJECT_TRIGGER, EVENT_STATUS_PROBLEM);

	if (ZBX_DB_OK > res)
		return FAIL;

	return SUCCEED;
}

static int	DBpatch_3050009(void)
{
	int		res;
	char		*trdefault = (char *)ZBX_DEFAULT_INTERNAL_TRIGGER_EVENT_NAME;

	if (0 == (program_type & ZBX_PROGRAM_TYPE_SERVER))
		return SUCCEED;

	res = DBexecute("update problem set name='%s' where source=%d and object=%d ", trdefault,
			EVENT_SOURCE_INTERNAL, EVENT_OBJECT_TRIGGER);

	if (ZBX_DB_OK > res)
		return FAIL;

	return SUCCEED;
}

static int	DBpatch_3050010(void)
{
	int		res;
	char		*itdefault = (char *)ZBX_DEFAULT_INTERNAL_ITEM_EVENT_NAME;

	if (0 == (program_type & ZBX_PROGRAM_TYPE_SERVER))
		return SUCCEED;

	res = DBexecute("update events set name='%s' where source=%d and object=%d and value=%d", itdefault,
			EVENT_SOURCE_INTERNAL, EVENT_OBJECT_ITEM, EVENT_STATUS_PROBLEM);

	if (ZBX_DB_OK > res)
		return FAIL;

	return SUCCEED;
}

static int	DBpatch_3050011(void)
{
	int		res;
	char		*itdefault = (char *)ZBX_DEFAULT_INTERNAL_ITEM_EVENT_NAME;

	if (0 == (program_type & ZBX_PROGRAM_TYPE_SERVER))
		return SUCCEED;

	res = DBexecute("update problem set name='%s' where source=%d and object=%d", itdefault,
			EVENT_SOURCE_INTERNAL, EVENT_OBJECT_ITEM);

	if (ZBX_DB_OK > res)
		return FAIL;

	return SUCCEED;
}

static int	DBpatch_3050012(void)
{
	int		res;

	if (0 == (program_type & ZBX_PROGRAM_TYPE_SERVER))
		return SUCCEED;

	res = DBexecute("update profiles set idx='web.problem.filter.name' where idx='web.problem.filter.problem'");

	if (ZBX_DB_OK > res)
		return FAIL;

	return SUCCEED;
}

static int	DBpatch_3050013(void)
{
	const ZBX_FIELD	field = {"dns", "", NULL, NULL, 255, ZBX_TYPE_CHAR, ZBX_NOTNULL, 0};

	return DBmodify_field_type("interface", &field, NULL);
}

static int	DBpatch_3050014(void)
{
	const ZBX_FIELD	field = {"dns", "", NULL, NULL, 255, ZBX_TYPE_CHAR, ZBX_NOTNULL, 0};

	return DBmodify_field_type("proxy_dhistory", &field, NULL);
}

static int	DBpatch_3050015(void)
{
	const ZBX_FIELD	field = {"listen_dns", "", NULL, NULL, 255, ZBX_TYPE_CHAR, ZBX_NOTNULL, 0};

	return DBmodify_field_type("autoreg_host", &field, NULL);
}

static int	DBpatch_3050016(void)
{
	const ZBX_FIELD	field = {"listen_dns", "", NULL, NULL, 255, ZBX_TYPE_CHAR, ZBX_NOTNULL, 0};

	return DBmodify_field_type("proxy_autoreg_host", &field, NULL);
}

static int	DBpatch_3050017(void)
{
	const ZBX_FIELD	field = {"dns", "", NULL, NULL, 255, ZBX_TYPE_CHAR, ZBX_NOTNULL, 0};

	return DBmodify_field_type("dservices", &field, NULL);
}

static int	DBpatch_3050018(void)
{
	return DBdrop_table("graph_theme");
}

static int	DBpatch_3050019(void)
{
	const ZBX_TABLE table =
		{"graph_theme",	"graphthemeid",	0,
			{
				{"graphthemeid", NULL, NULL, NULL, 0, ZBX_TYPE_ID, ZBX_NOTNULL, 0},
				{"theme", "", NULL, NULL, 64, ZBX_TYPE_CHAR, ZBX_NOTNULL, 0},
				{"backgroundcolor", "", NULL, NULL, 6, ZBX_TYPE_CHAR, ZBX_NOTNULL, 0},
				{"graphcolor", "", NULL, NULL, 6, ZBX_TYPE_CHAR, ZBX_NOTNULL, 0},
				{"gridcolor", "", NULL, NULL, 6, ZBX_TYPE_CHAR, ZBX_NOTNULL, 0},
				{"maingridcolor", "", NULL, NULL, 6, ZBX_TYPE_CHAR, ZBX_NOTNULL, 0},
				{"gridbordercolor", "", NULL, NULL, 6, ZBX_TYPE_CHAR, ZBX_NOTNULL, 0},
				{"textcolor", "", NULL, NULL, 6, ZBX_TYPE_CHAR, ZBX_NOTNULL, 0},
				{"highlightcolor", "", NULL, NULL, 6, ZBX_TYPE_CHAR, ZBX_NOTNULL, 0},
				{"leftpercentilecolor", "", NULL, NULL, 6, ZBX_TYPE_CHAR, ZBX_NOTNULL, 0},
				{"rightpercentilecolor", "", NULL, NULL, 6, ZBX_TYPE_CHAR, ZBX_NOTNULL, 0},
				{"nonworktimecolor", "", NULL, NULL, 6, ZBX_TYPE_CHAR, ZBX_NOTNULL, 0},
				{"colorpalette", "", NULL, NULL, 255, ZBX_TYPE_CHAR, ZBX_NOTNULL, 0},
				{0}
			},
			NULL
		};

	return DBcreate_table(&table);
}

static int	DBpatch_3050020(void)
{
	return DBcreate_index("graph_theme", "graph_theme_1", "theme", 1);
}

#define ZBX_COLORPALETTE_LIGHT	"1A7C11,F63100,2774A4,A54F10,FC6EA3,6C59DC,AC8C14,611F27,F230E0,5CCD18,BB2A02,"	\
				"5A2B57,89ABF8,7EC25C,274482,2B5429,8048B4,FD5434,790E1F,87AC4D,E89DF4"
#define ZBX_COLORPALETTE_DARK	"199C0D,F63100,2774A4,F7941D,FC6EA3,6C59DC,C7A72D,BA2A5D,F230E0,5CCD18,BB2A02,"	\
				"AC41A5,89ABF8,7EC25C,3165D5,79A277,AA73DE,FD5434,F21C3E,87AC4D,E89DF4"

static int	DBpatch_3050021(void)
{
	if (0 == (ZBX_PROGRAM_TYPE_SERVER & program_type))
		return SUCCEED;

	if (ZBX_DB_OK <= DBexecute(
			"insert into graph_theme"
			" values (1,'blue-theme','FFFFFF','FFFFFF','CCD5D9','ACBBC2','ACBBC2','1F2C33','E33734',"
				"'429E47','E33734','EBEBEB','" ZBX_COLORPALETTE_LIGHT "')"))
	{
		return SUCCEED;
	}

	return FAIL;
}

static int	DBpatch_3050022(void)
{
	if (0 == (ZBX_PROGRAM_TYPE_SERVER & program_type))
		return SUCCEED;

	if (ZBX_DB_OK <= DBexecute(
			"insert into graph_theme"
			" values (2,'dark-theme','2B2B2B','2B2B2B','454545','4F4F4F','4F4F4F','F2F2F2','E45959',"
				"'59DB8F','E45959','333333','" ZBX_COLORPALETTE_DARK "')"))
	{
		return SUCCEED;
	}

	return FAIL;
}

static int	DBpatch_3050023(void)
{
	if (0 == (ZBX_PROGRAM_TYPE_SERVER & program_type))
		return SUCCEED;

	if (ZBX_DB_OK <= DBexecute(
			"insert into graph_theme"
			" values (3,'hc-light','FFFFFF','FFFFFF','555555','000000','333333','000000','333333',"
				"'000000','000000','EBEBEB','" ZBX_COLORPALETTE_LIGHT "')"))
	{
		return SUCCEED;
	}

	return FAIL;
}

static int	DBpatch_3050024(void)
{
	if (0 == (ZBX_PROGRAM_TYPE_SERVER & program_type))
		return SUCCEED;

	if (ZBX_DB_OK <= DBexecute(
			"insert into graph_theme"
			" values (4,'hc-dark','000000','000000','666666','888888','4F4F4F','FFFFFF','FFFFFF',"
				"'FFFFFF','FFFFFF','333333','" ZBX_COLORPALETTE_DARK "')"))
	{
		return SUCCEED;
	}

	return FAIL;
}

#undef ZBX_COLORPALETTE_LIGHT
#undef ZBX_COLORPALETTE_DARK

static int	DBpatch_3050025(void)
{
	zbx_db_insert_t	db_insert;
	int		ret;

	if (0 == (program_type & ZBX_PROGRAM_TYPE_SERVER))
		return SUCCEED;

	zbx_db_insert_prepare(&db_insert, "task", "taskid", "type", "status", "clock", NULL);
	zbx_db_insert_add_values(&db_insert, __UINT64_C(0), ZBX_TM_TASK_UPDATE_EVENTNAMES, ZBX_TM_STATUS_NEW,
			time(NULL));
	zbx_db_insert_autoincrement(&db_insert, "taskid");
	ret = zbx_db_insert_execute(&db_insert);
	zbx_db_insert_clean(&db_insert);

	return ret;
}

static int	DBpatch_3050026(void)
{
	int	res;

	if (0 == (program_type & ZBX_PROGRAM_TYPE_SERVER))
		return SUCCEED;

	res = DBexecute("update profiles set value_str='name' where idx='web.problem.sort' and value_str='problem'");

	if (ZBX_DB_OK > res)
		return FAIL;

	return SUCCEED;
}

static int	DBpatch_3050027(void)
{
	const ZBX_FIELD	field = {"sendto", "", NULL, NULL, 1024, ZBX_TYPE_CHAR, ZBX_NOTNULL, 0};

	return DBmodify_field_type("media", &field, NULL);
}

static int	DBpatch_3050028(void)
{
	const ZBX_FIELD	field = {"sendto", "", NULL, NULL, 1024, ZBX_TYPE_CHAR, ZBX_NOTNULL, 0};

	return DBmodify_field_type("alerts", &field, NULL);
}

extern int	DBpatch_3040006(void);

static int	DBpatch_3050029(void)
{
	return DBpatch_3040006();
}

static int	DBpatch_3050030(void)
{
	const ZBX_FIELD	field = {"custom_color", "0", NULL, NULL, 0, ZBX_TYPE_INT, ZBX_NOTNULL, 0};

	return DBadd_field("config", &field);
}

static int	DBpatch_3050031(void)
{
	const ZBX_FIELD	field = {"problem_unack_color", "CC0000", NULL, NULL, 6, ZBX_TYPE_CHAR, ZBX_NOTNULL, 0};

	return DBset_default("config", &field);
}

static int	DBpatch_3050032(void)
{
	const ZBX_FIELD	field = {"problem_ack_color", "CC0000", NULL, NULL, 6, ZBX_TYPE_CHAR, ZBX_NOTNULL, 0};

	return DBset_default("config", &field);
}

static int	DBpatch_3050033(void)
{
	const ZBX_FIELD	field = {"ok_unack_color", "009900", NULL, NULL, 6, ZBX_TYPE_CHAR, ZBX_NOTNULL, 0};

	return DBset_default("config", &field);
}

static int	DBpatch_3050034(void)
{
	const ZBX_FIELD	field = {"ok_ack_color", "009900", NULL, NULL, 6, ZBX_TYPE_CHAR, ZBX_NOTNULL, 0};

	return DBset_default("config", &field);
}

static int	DBpatch_3050035(void)
{
	int	res;

	res = DBexecute(
		"update config"
		" set custom_color=1"
		" where problem_unack_color<>'DC0000'"
			" or problem_ack_color<>'DC0000'"
			" or ok_unack_color<>'00AA00'"
			" or ok_ack_color<>'00AA00'");

	if (ZBX_DB_OK > res)
		return FAIL;

	return SUCCEED;
}

static int	DBpatch_3050036(void)
{
	int	res;

	res = DBexecute(
		"update config"
		" set problem_unack_color='CC0000',"
			"problem_ack_color='CC0000',"
			"ok_unack_color='009900',"
			"ok_ack_color='009900'"
		" where problem_unack_color='DC0000'"
			" and problem_ack_color='DC0000'"
			" and ok_unack_color='00AA00'"
			" and ok_ack_color='00AA00'");

	if (ZBX_DB_OK > res)
		return FAIL;

	return SUCCEED;
}

extern int	DBpatch_3040007(void);

static int	DBpatch_3050037(void)
{
	return DBpatch_3040007();
}

static int	DBpatch_3050038(void)
{
	const ZBX_TABLE table =
			{"tag_filter", "tag_filterid", 0,
				{
					{"tag_filterid", NULL, NULL, NULL, 0, ZBX_TYPE_ID, ZBX_NOTNULL, 0},
					{"usrgrpid", NULL, NULL, NULL, 0, ZBX_TYPE_ID, ZBX_NOTNULL, 0},
					{"groupid", NULL, NULL, NULL, 0, ZBX_TYPE_ID, ZBX_NOTNULL, 0},
					{"tag", "", NULL, NULL, 255, ZBX_TYPE_CHAR, ZBX_NOTNULL, 0},
					{"value", "", NULL, NULL, 255, ZBX_TYPE_CHAR, ZBX_NOTNULL, 0},
					{0}
				},
				NULL
			};

	return DBcreate_table(&table);
}

static int	DBpatch_3050039(void)
{
	const ZBX_FIELD	field = {"usrgrpid", NULL, "usrgrp", "usrgrpid", 0, 0, 0, ZBX_FK_CASCADE_DELETE};

	return DBadd_foreign_key("tag_filter", 1, &field);
}

static int	DBpatch_3050040(void)
{
	const ZBX_FIELD	field = {"groupid", NULL, "groups", "groupid", 0, 0, 0, ZBX_FK_CASCADE_DELETE};

	return DBadd_foreign_key("tag_filter", 2, &field);
}

static int	DBpatch_3050041(void)
{
	const ZBX_TABLE table =
			{"task_check_now", "taskid", 0,
				{
					{"taskid", NULL, NULL, NULL, 0, ZBX_TYPE_ID, ZBX_NOTNULL, 0},
					{"itemid", NULL, NULL, NULL, 0, ZBX_TYPE_ID, ZBX_NOTNULL, 0},
					{0}
				},
				NULL
			};

	return DBcreate_table(&table);
}

static int	DBpatch_3050042(void)
{
	const ZBX_FIELD	field = {"taskid", NULL, "task", "taskid", 0, 0, 0, ZBX_FK_CASCADE_DELETE};

	return DBadd_foreign_key("task_check_now", 1, &field);
}

static int	DBpatch_3050043(void)
{
	const char	*sql =
		"update widget_field"
		" set value_int=3"
		" where name='show_tags'"
			" and exists ("
				"select null"
				" from widget w"
				" where widget_field.widgetid=w.widgetid"
					" and w.type='problems'"
			")";

	if (0 == (program_type & ZBX_PROGRAM_TYPE_SERVER))
		return SUCCEED;

	if (ZBX_DB_OK <= DBexecute("%s", sql))
		return SUCCEED;

	return FAIL;
}

static int	DBpatch_3050044(void)
{
	const char	*sql =
		"delete from profiles"
		" where idx in ('web.paging.lastpage','web.menu.view.last') and value_str='tr_status.php'"
			" or idx like 'web.tr_status%'";

	if (0 == (program_type & ZBX_PROGRAM_TYPE_SERVER))
		return SUCCEED;

	if (ZBX_DB_OK <= DBexecute("%s", sql))
		return SUCCEED;

	return FAIL;
}

static int	DBpatch_3050045(void)
{
	const char	*sql = "update users set url='zabbix.php?action=problem.view' where url like '%tr_status.php%'";

	if (0 == (program_type & ZBX_PROGRAM_TYPE_SERVER))
		return SUCCEED;

	if (ZBX_DB_OK <= DBexecute("%s", sql))
		return SUCCEED;

	return FAIL;
}

static int	DBpatch_3050046(void)
{
	const ZBX_FIELD field = {"timeout", "3s", NULL, NULL, 255, ZBX_TYPE_CHAR, ZBX_NOTNULL | ZBX_PROXY, 0};

	return DBadd_field("items", &field);
}

static int	DBpatch_3050047(void)
{
	const ZBX_FIELD field = {"url", "", NULL, NULL, 2048, ZBX_TYPE_CHAR, ZBX_NOTNULL | ZBX_PROXY, 0};

	return DBadd_field("items", &field);
}

static int	DBpatch_3050048(void)
{
	const ZBX_FIELD field = {"query_fields", "", NULL, NULL, 2048, ZBX_TYPE_CHAR, ZBX_NOTNULL | ZBX_PROXY, 0};

	return DBadd_field("items", &field);
}

static int	DBpatch_3050049(void)
{
	const ZBX_FIELD	field = {"posts", "", NULL, NULL, 0, ZBX_TYPE_SHORTTEXT, ZBX_NOTNULL | ZBX_PROXY, 0};

	return DBadd_field("items", &field);
}

static int	DBpatch_3050050(void)
{
	const ZBX_FIELD field = {"status_codes", "200", NULL, NULL, 255, ZBX_TYPE_CHAR, ZBX_NOTNULL | ZBX_PROXY, 0};

	return DBadd_field("items", &field);
}

static int	DBpatch_3050051(void)
{
	const ZBX_FIELD field = {"follow_redirects", "1", NULL, NULL, 0, ZBX_TYPE_INT, ZBX_NOTNULL | ZBX_PROXY, 0};

	return DBadd_field("items", &field);
}

static int	DBpatch_3050052(void)
{
	const ZBX_FIELD field = {"post_type", "0", NULL, NULL, 0, ZBX_TYPE_INT, ZBX_NOTNULL | ZBX_PROXY, 0};

	return DBadd_field("items", &field);
}

static int	DBpatch_3050053(void)
{
	const ZBX_FIELD field = {"http_proxy", "", NULL, NULL, 255, ZBX_TYPE_CHAR, ZBX_NOTNULL | ZBX_PROXY, 0};

	return DBadd_field("items", &field);
}

static int	DBpatch_3050054(void)
{
	const ZBX_FIELD	field = {"headers", "", NULL, NULL, 0, ZBX_TYPE_SHORTTEXT, ZBX_NOTNULL | ZBX_PROXY, 0};

	return DBadd_field("items", &field);
}

static int	DBpatch_3050055(void)
{
	const ZBX_FIELD field = {"retrieve_mode", "0", NULL, NULL, 0, ZBX_TYPE_INT, ZBX_NOTNULL | ZBX_PROXY, 0};

	return DBadd_field("items", &field);
}

static int	DBpatch_3050056(void)
{
	const ZBX_FIELD field = {"request_method", "1", NULL, NULL, 0, ZBX_TYPE_INT, ZBX_NOTNULL | ZBX_PROXY, 0};

	return DBadd_field("items", &field);
}

static int	DBpatch_3050057(void)
{
	const ZBX_FIELD field = {"output_format", "0", NULL, NULL, 0, ZBX_TYPE_INT, ZBX_NOTNULL | ZBX_PROXY, 0};

	return DBadd_field("items", &field);
}

static int	DBpatch_3050058(void)
{
	const ZBX_FIELD field = {"ssl_cert_file", "", NULL, NULL, 255, ZBX_TYPE_CHAR, ZBX_NOTNULL | ZBX_PROXY, 0};

	return DBadd_field("items", &field);
}

static int	DBpatch_3050059(void)
{
	const ZBX_FIELD field = {"ssl_key_file", "", NULL, NULL, 255, ZBX_TYPE_CHAR, ZBX_NOTNULL | ZBX_PROXY, 0};

	return DBadd_field("items", &field);
}

static int	DBpatch_3050060(void)
{
	const ZBX_FIELD field = {"ssl_key_password", "", NULL, NULL, 64, ZBX_TYPE_CHAR, ZBX_NOTNULL | ZBX_PROXY, 0};

	return DBadd_field("items", &field);
}

static int	DBpatch_3050061(void)
{
	const ZBX_FIELD field = {"verify_peer", "0", NULL, NULL, 0, ZBX_TYPE_INT, ZBX_NOTNULL | ZBX_PROXY, 0};

	return DBadd_field("items", &field);
}

static int	DBpatch_3050062(void)
{
	const ZBX_FIELD field = {"verify_host", "0", NULL, NULL, 0, ZBX_TYPE_INT, ZBX_NOTNULL | ZBX_PROXY, 0};

	return DBadd_field("items", &field);
}

static int	DBpatch_3050063(void)
{
	const ZBX_FIELD field = {"allow_traps", "0", NULL, NULL, 0, ZBX_TYPE_INT, ZBX_NOTNULL | ZBX_PROXY, 0};

	return DBadd_field("items", &field);
}

static int	DBpatch_3050064(void)
{
	const ZBX_FIELD	field = {"auto_compress", "1", NULL, NULL, 0, ZBX_TYPE_INT, ZBX_NOTNULL, 0};

	return DBadd_field("hosts", &field);
}

static int	DBpatch_3050065(void)
{
	int	ret;

	if (0 == (program_type & ZBX_PROGRAM_TYPE_SERVER))
		return SUCCEED;

	/* 5 - HOST_STATUS_PROXY_ACTIVE, 6 - HOST_STATUS_PROXY_PASSIVE */
	ret = DBexecute("update hosts set auto_compress=0 where status=5 or status=6");

	if (ZBX_DB_OK > ret)
		return FAIL;

	return SUCCEED;
}

static int	DBpatch_3050066(void)
{
	int		i;
	const char      *types[] = {
			"actlog", "actionlog",
			"dscvry", "discovery",
			"favgrph", "favgraphs",
			"favmap", "favmaps",
			"favscr", "favscreens",
			"hoststat", "problemhosts",
			"navigationtree", "navtree",
			"stszbx", "systeminfo",
			"sysmap", "map",
			"syssum", "problemsbysv",
			"webovr", "web",
			NULL
		};

	if (0 == (program_type & ZBX_PROGRAM_TYPE_SERVER))
		return SUCCEED;

	for (i = 0; NULL != types[i]; i += 2)
	{
		if (ZBX_DB_OK > DBexecute("update widget set type='%s' where type='%s'", types[i + 1], types[i]))
			return FAIL;
	}

	return SUCCEED;
}

static int	DBpatch_3050067(void)
{
	return DBdrop_field("config", "event_expire");
}

static int	DBpatch_3050068(void)
{
	return DBdrop_field("config", "event_show_max");
}

#endif

DBPATCH_START(3050)

/* version, duplicates flag, mandatory flag */

DBPATCH_ADD(3050000, 0, 1)
DBPATCH_ADD(3050001, 0, 1)
DBPATCH_ADD(3050004, 0, 1)
DBPATCH_ADD(3050005, 0, 1)
DBPATCH_ADD(3050008, 0, 1)
DBPATCH_ADD(3050009, 0, 1)
DBPATCH_ADD(3050010, 0, 1)
DBPATCH_ADD(3050011, 0, 1)
DBPATCH_ADD(3050012, 0, 1)
DBPATCH_ADD(3050013, 0, 1)
DBPATCH_ADD(3050014, 0, 1)
DBPATCH_ADD(3050015, 0, 1)
DBPATCH_ADD(3050016, 0, 1)
DBPATCH_ADD(3050017, 0, 1)
DBPATCH_ADD(3050018, 0, 1)
DBPATCH_ADD(3050019, 0, 1)
DBPATCH_ADD(3050020, 0, 1)
DBPATCH_ADD(3050021, 0, 1)
DBPATCH_ADD(3050022, 0, 1)
DBPATCH_ADD(3050023, 0, 1)
DBPATCH_ADD(3050024, 0, 1)
DBPATCH_ADD(3050025, 0, 1)
DBPATCH_ADD(3050026, 0, 1)
DBPATCH_ADD(3050027, 0, 1)
DBPATCH_ADD(3050028, 0, 1)
DBPATCH_ADD(3050029, 0, 0)
DBPATCH_ADD(3050030, 0, 1)
DBPATCH_ADD(3050031, 0, 1)
DBPATCH_ADD(3050032, 0, 1)
DBPATCH_ADD(3050033, 0, 1)
DBPATCH_ADD(3050034, 0, 1)
DBPATCH_ADD(3050035, 0, 1)
DBPATCH_ADD(3050036, 0, 1)
DBPATCH_ADD(3050037, 0, 1)
DBPATCH_ADD(3050038, 0, 1)
DBPATCH_ADD(3050039, 0, 1)
DBPATCH_ADD(3050040, 0, 1)
DBPATCH_ADD(3050041, 0, 1)
DBPATCH_ADD(3050042, 0, 1)
DBPATCH_ADD(3050043, 0, 1)
DBPATCH_ADD(3050044, 0, 1)
DBPATCH_ADD(3050045, 0, 1)
DBPATCH_ADD(3050046, 0, 1)
DBPATCH_ADD(3050047, 0, 1)
DBPATCH_ADD(3050048, 0, 1)
DBPATCH_ADD(3050049, 0, 1)
DBPATCH_ADD(3050050, 0, 1)
DBPATCH_ADD(3050051, 0, 1)
DBPATCH_ADD(3050052, 0, 1)
DBPATCH_ADD(3050053, 0, 1)
DBPATCH_ADD(3050054, 0, 1)
DBPATCH_ADD(3050055, 0, 1)
DBPATCH_ADD(3050056, 0, 1)
DBPATCH_ADD(3050057, 0, 1)
DBPATCH_ADD(3050058, 0, 1)
DBPATCH_ADD(3050059, 0, 1)
DBPATCH_ADD(3050060, 0, 1)
DBPATCH_ADD(3050061, 0, 1)
DBPATCH_ADD(3050062, 0, 1)
DBPATCH_ADD(3050063, 0, 1)
DBPATCH_ADD(3050064, 0, 1)
DBPATCH_ADD(3050065, 0, 1)
DBPATCH_ADD(3050066, 0, 1)
DBPATCH_ADD(3050067, 0, 1)
DBPATCH_ADD(3050068, 0, 1)

DBPATCH_END()
