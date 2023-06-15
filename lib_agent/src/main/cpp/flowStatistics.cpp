#include "flowStatistics.h"
#include <stdlib.h>
#include <string.h>
#include "zxlog.h"

#define DEFAULT_STATISTICAL_INTERVAL 10

void flow_statistics_clear(flow_statistics_t * flow_statistics, zxuint64 time);


flow_second_t * create_flow_second(zxuint64 time, zxuint64 byte_count)
{
	flow_second_t *flow_second = (flow_second_t *)malloc(sizeof(flow_second_t));
	if (NULL == flow_second) return NULL;

	flow_second->time = time;
	flow_second->bytes = byte_count;

	return flow_second;
}

flow_statistics_t * create_flow_statistics()
{
	return create_flow_statistics(DEFAULT_STATISTICAL_INTERVAL);
}

flow_statistics_t * create_flow_statistics(int statistical_interval)
{
	flow_statistics_t *flow_statistics = (flow_statistics_t *)malloc(sizeof(_flow_statistics_t));
	if (NULL == flow_statistics) return NULL;

	init_list_head(&flow_statistics->flow_read);
	init_list_head(&flow_statistics->flow_write);

	flow_statistics->total_read_bytes = flow_statistics->total_write_bytes = 0;
	
	flow_statistics->statistical_interval = statistical_interval;

	return flow_statistics;
}

void flow_statistics_release(flow_statistics_t *fs)
{
	list_head *pos = NULL;

	list_for_each(pos, &fs->flow_read) {
		flow_second_t *fs_tmp = list_entry(pos, flow_second_t, node);
		list_del(pos);
		pos = pos->prev;	//不使用前指针，del后pos->next为野指针
		free(fs_tmp);
	}

	list_for_each(pos, &fs->flow_write) {
		flow_second_t *fs_tmp = list_entry(pos, flow_second_t, node);
		list_del(pos);
		pos = pos->prev;	//不使用前指针，del后pos->next为野指针
		free(fs_tmp);
	}

	free(fs);
}

void add_read_bytes(flow_statistics_t * flow_statistics, zxuint64 ntime, unsigned int byte_count)
{
	list_head *pos = NULL;
	flow_second_t *flow_second = NULL;

	flow_statistics_clear(flow_statistics, ntime);

	// TODO：list_last_entry 优化一下
	list_for_each(pos, &flow_statistics->flow_read) {
		flow_second_t *fs_tmp = list_entry(pos, flow_second_t, node);
		if (fs_tmp->time == ntime) {
			flow_second = fs_tmp;
			
			break;
		}
	}

	if (NULL == flow_second) {
		flow_second = create_flow_second(ntime, byte_count);
		if (NULL == flow_second) {
			return;
		}
		list_add_tail(&flow_second->node, &flow_statistics->flow_read);
	}
	else {
		flow_second->bytes += byte_count;
	}

	flow_statistics->total_read_bytes += byte_count;
}

void add_write_bytes(flow_statistics_t * flow_statistics, zxuint64 ntime, unsigned int byte_count)
{
	list_head *pos = NULL;
	flow_second_t *flow_second = NULL;

	flow_statistics_clear(flow_statistics, ntime);

	// TODO：list_last_entry 优化一下
	list_for_each(pos, &flow_statistics->flow_write) {
		flow_second_t *fs_tmp = list_entry(pos, flow_second_t, node);
		if (fs_tmp->time == ntime) {
			flow_second = fs_tmp;
			break;
		}
	}

	if (NULL == flow_second) {
		flow_second = create_flow_second(ntime, byte_count);
		if (NULL == flow_second) {
			return;
		}
		list_add_tail(&flow_second->node, &flow_statistics->flow_write);
	}
	else {
		flow_second->bytes += byte_count;
	}

	flow_statistics->total_write_bytes += byte_count;
}

zxuint64 get_read_bytes_with_statistical_interval(flow_statistics_t * flow_statistics, zxuint64 time)
{
	flow_statistics_clear(flow_statistics, time);
	return flow_statistics->total_read_bytes;
}

zxuint64 get_write_bytes_with_statistical_interval(flow_statistics_t * flow_statistics, zxuint64 time)
{
	flow_statistics_clear(flow_statistics, time);
	return flow_statistics->total_write_bytes;
}

void flow_statistics_clear(flow_statistics_t * flow_statistics, zxuint64 ntime)
{
	// 清理超过统计时间的流量节点，下行节点
	do
	{
		if (list_is_empty(&flow_statistics->flow_read)) {
			break;
		}

		flow_second_t *flow_second = list_first_entry(&flow_statistics->flow_read, flow_second_t, node);
		
		if (flow_second->time > ntime - flow_statistics->statistical_interval) {
			break;
		}

		list_del(&flow_second->node);
		flow_statistics->total_read_bytes -= flow_second->bytes;		
		free(flow_second);
	} while (true);

	// 清理超过统计时间的流量节点，上行节点
	do
	{
		if (list_is_empty(&flow_statistics->flow_write)) {
			break;
		}

		flow_second_t *flow_second = list_first_entry(&flow_statistics->flow_write, flow_second_t, node);
		
		if (flow_second->time > ntime - flow_statistics->statistical_interval) {
			break;
		}

		list_del(&flow_second->node);
		flow_statistics->total_write_bytes -= flow_second->bytes;
		//LOGE("free write %llu", flow_second->bytes);
		free(flow_second);
	} while (true);
}