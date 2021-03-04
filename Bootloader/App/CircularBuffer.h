/*
 * CircularBuffer.h
 *
 *  Created on: 2 мар. 2021 г.
 *      Author: Evgeny
 */

#pragma once

template<typename DataType, size_t size = 4>
struct CircularBuffer {

	bool put(const DataType& value){
		size_t next = nextPos(_last);
		if (next == _first) return false;
		_buffer[_last] = value;
		_last = next;
		return true;
	}

	bool get(DataType& value){
		if (_first == _last) return false;
		value = _buffer[_first];
		_first = nextPos(_first);
		return true;
	}

	bool getButNotTake(DataType& value){
		if (_first == _last) return false;
		value = _buffer[_first];
		return true;
	}

	bool isEmpty(){
		return (_first == _last);
	}

private:
	size_t nextPos(size_t current){
		register size_t next = current + 1;
		if (next >= size) next = 0;
		return next;
	}

private:
	volatile DataType	_buffer[size] {};
	volatile size_t		_first = 0;
	volatile size_t		_last = 0;
};
