#ifndef LFU_H
#define LFU_H

#include <iostream>
#include <list>
#include <map>
#include <ctime>
#include <cassert>

template <typename KEY, typename VALUE>
class CLFU
{
	struct SValueInfo
	{
		size_t mFreq;
		size_t mWeight;
		KEY mKey;
		VALUE mVal;
	};

	typedef typename std::list<SValueInfo>::iterator	ListIter;
	typedef typename std::map<KEY, ListIter>::iterator	MapIter;
	typedef typename std::map<KEY, ListIter>			Map;
	typedef typename std::list<SValueInfo>				List;

public:
	CLFU(size_t _weight, std::time_t _timeout = 30) : 
		mMaxWeight(_weight), 
		mTimeStamp(std::time(0)),
		mTimeOut(_timeout)
	{}

	// 测试使用
	void print()
	{
		int last_freq = 0;
		for (ListIter iter_begin = mVals.begin(); iter_begin != mVals.end(); ++iter_begin) {
			std::cout << iter_begin->mVal << "(" << iter_begin->mFreq << ")" << ", ";
			assert(last_freq <= iter_begin->mFreq);
			last_freq = iter_begin->mFreq;
		}
		std::cout << std::endl;
	}

	VALUE get(KEY _key, size_t _freq = 1)
	{
		// 找不到该key，返回0
		MapIter map_iter = mIndex.find(_key);
		if (map_iter == mIndex.end()) {
			return 0;
		}

		ListIter list_iter = map_iter->second;
		list_iter->mFreq += _freq;

		// 重新排序
		sort(list_iter);

		return list_iter->mVal;
	}

	void put(KEY _key, VALUE _val, size_t _weight = 1, size_t _freq = 1)
	{
		// 如果已经存在该key, 不更新值直接返回
		MapIter map_iter = mIndex.find(_key);
		if (map_iter != mIndex.end()) {
			return;
		}

		// 更新当前总权重
		mCurWeight += _weight;

		// 如果当前总权重大于最大总权重，则依次删除频率最低的数据直到当前总权重小于等于最大总权重
		while (mCurWeight > mMaxWeight) {
			pop();
		}

		// 构造新数据
		SValueInfo _info;
		_info.mFreq = _freq;
		_info.mWeight = _weight;
		_info.mKey = _key;
		_info.mVal = _val;

		// 添加新数据
		push(_info);
	}

private:
	// 开头删除一个数据
	void pop()
	{
		ListIter last_iter = mVals.begin();
		mCurWeight -= last_iter->mWeight;
		mIndex.erase(last_iter->mKey);
		mVals.pop_front();
	}

	/*
		@brief 添加数据到合适的位置
	*/
	void push(const SValueInfo &_info)
	{
		// 找到该频率在链表中的位置(如果有多个相同频率，则找到最前那个频率)
		ListIter _iter = find(mVals.begin(), _info.mFreq);

		// 插入到数据链表
		ListIter new_iter = mVals.insert(_iter, _info);

		// 插入到查询表
		mIndex[_info.mKey] = new_iter;
	}

	/*
		@brief 重新排序
	*/
	void sort(ListIter &_iter)
	{
		// 整体移位
		decreaseFreq();

		// 找到比当前频率大的前一个节点
		// 如从链表中[v1(4)，v2(3)，v3(3)，v4(2)，v5(2)，v6(1)] 找频率为2的，则返回v4(2)
		ListIter prev_iter = find(_iter, _iter->mFreq);
		if (_iter == prev_iter) {
			return;
		}

		// 重新排序
		mVals.splice(prev_iter, mVals, _iter);
	}

	/*
		@breif 从_from开始往回找到最后一个freq不比_freq大的节点
	*/
	ListIter find(ListIter _from, size_t _freq)
	{
		for (; _from != mVals.end(); ++_from) {
			// 如果当前频率大于查询频率
			if (_from->mFreq > _freq) {
				return _from;
			}
		}

		return mVals.end();
	}

	// 返回超时移动的位数
	int timeout()
	{
		std::time_t cur_t = std::time(0);
		std::time_t interval_t = cur_t - mTimeStamp;
		if (interval_t < mTimeOut) {
			return 0;
		}

		mTimeStamp = cur_t;
		int _move = interval_t / mTimeOut;
		return _move >= 32 ? 31 : _move;
	}

	// 所有频率右移一位
	void decreaseFreq()
	{
		// 返回右移位数
		int dec = timeout();
		if (dec == 0) {
			return;
		}

		std::cout << "decrease a bit" << std::endl; // 测试使用

		// 整体减少频率
		for (ListIter iter_begin = mVals.begin(); iter_begin != mVals.end(); ++iter_begin) {
			iter_begin->mFreq >>= dec;
		}
	}

private:
	std::time_t mTimeOut;
	std::time_t	mTimeStamp;
	size_t mMaxWeight;
	size_t mCurWeight;
	std::map<KEY, ListIter> mIndex;
	std::list<SValueInfo> mVals;
};

#endif
