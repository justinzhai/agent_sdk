#pragma once

#include "zxlist.h"
#include "zxstruct.h"


typedef struct _flow_statistics_t flow_statistics_t;
typedef struct _flow_second_t flow_second_t;

struct _flow_second_t
{
	struct list_head node;	// 链表
	zxuint64 time;			//时间
	zxuint64 bytes;		//该单位时间内总计的流量
};

struct _flow_statistics_t
{
	struct list_head flow_read;
	struct list_head flow_write;
	zxuint64 total_read_bytes;
	zxuint64 total_write_bytes;
	int statistical_interval;		//统计区间, 单位秒
};

flow_second_t *create_flow_second(zxuint64 time, zxuint64 byte_count);

flow_statistics_t *create_flow_statistics();
flow_statistics_t *create_flow_statistics(int statistical_interval);
void flow_statistics_release(flow_statistics_t *fs);

void add_read_bytes(flow_statistics_t *flow_statistics, zxuint64 time, unsigned int byte_count);
void add_write_bytes(flow_statistics_t *flow_statistics, zxuint64 time, unsigned int byte_count);

zxuint64 get_read_bytes_with_statistical_interval(flow_statistics_t *flow_statistics, zxuint64 time);
zxuint64 get_write_bytes_with_statistical_interval(flow_statistics_t *flow_statistics, zxuint64 time);