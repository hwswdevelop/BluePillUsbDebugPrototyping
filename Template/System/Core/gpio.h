/*
 * Gpio.h
 *
 *  Created on: Mar 6, 2021
 *      Author: Evgeny
 */

#pragma once

#include <cstdint>

struct GPIO {
	volatile uint32_t CRL;
	volatile uint32_t CRH;
	volatile const uint32_t IDR;
	volatile uint32_t ODR;
	volatile uint32_t BSRR;
	volatile uint32_t BRR;
	volatile uint32_t LCKR;
};

namespace Gpio {
	namespace Pin {

		enum class Direction : uint32_t {
			Input,
			Output,
		};

		enum class InputMode : uint32_t {
			Inp
		};

		enum class OutputMode : uint32_t {
			PushPull,
			OpenDrain,
			OpenSource
		};

		enum class Pull : uint32_t {
			None,
			Up,
			Down
		};

		enum class Speed : uint32_t {
			Low,
			Medium,
			High
		};

		struct Id {
			constexpr Id(uint32_t GpioRegBase, uint32_t GpioPinNo) : RegBase(GpioRegBase), PinNo(GpioPinNo), PinMask(1 << PinNo) {};
			const uint32_t RegBase;
			const uint32_t PinNo;
			const uint32_t PinMask;
		};

		extern const Id pinIdByNo[];

		inline void set(const Id& id){
			reinterpret_cast<volatile GPIO*>(id.RegBase)->BSRR = ( 1 << id.PinNo );
		}

		inline void reset(const Id& id){
			reinterpret_cast<volatile GPIO*>(id.RegBase)->BRR = ( 1 << id.PinNo );
		}

		inline void invert(const Id& id){
			reinterpret_cast<volatile GPIO*>(id.RegBase)->ODR = ( 1 << id.PinNo );
		}

		inline void set(const uint32_t no){
			set( pinIdByNo[no] );
		}

		inline void reset(const uint32_t no){
			reset( pinIdByNo[no] );
		}

		inline void invert(const uint32_t no){
			invert( pinIdByNo[no] );
		}

	}
}

static constexpr const uint32_t GPIOA_BASE = 0x40010800;
static constexpr const uint32_t GPIOB_BASE = 0x40010C00;
static constexpr const uint32_t GPIOC_BASE = 0x40011000;

enum class GpioMode : uint8_t {
	InputAnalog = 0x00,
	InputFloating,
	InputWithPullup,
	OutputPushPull = 0x04,
	OutputOpenDrain,
	AlternatePushPull,
	AlternateOpenDrain
};

enum class GpioOutputSpeed : uint8_t {
	Input,
	Max10Mhz,
	Max2Mhz,
	Max50Mhz
};

template<uint32_t GpioAddr, uint8_t pinNo>
struct GpioPin {
	static constexpr const uint32_t GpioAddress = GpioAddr;
	static constexpr const uint8_t  GpioPinNo = pinNo;
	static constexpr const uint32_t GpioPinMask = (1 << pinNo);
	static constexpr const uint32_t GpioConfPerReg = 8;

	static inline void mode(const GpioMode mode, const GpioOutputSpeed oSpeed = GpioOutputSpeed::Input){
		if constexpr (GpioPinNo < GpioConfPerReg){
			static constexpr const uint32_t maskBitCount = 4;
			static constexpr const uint32_t maskOffset = (pinNo * maskBitCount) & 0x1F;
			static constexpr const uint32_t mask = (1 << (maskBitCount + 1)) - 1;
			reinterpret_cast<volatile GPIO* const>(GpioAddress)->CRL &= ~(mask << maskOffset);
			reinterpret_cast<volatile GPIO* const>(GpioAddress)->CRL |= ((static_cast<uint32_t>(mode) & 0x03) << maskOffset);
			reinterpret_cast<volatile GPIO* const>(GpioAddress)->CRL |= ((static_cast<uint32_t>(oSpeed) & 0x03) << (maskOffset + 2));
		} else {
			static constexpr const uint32_t maskBitCount = 4;
			static constexpr const uint32_t maskOffset = (pinNo * maskBitCount) & 0x1F;
			static constexpr const uint32_t mask = (1 << (maskBitCount + 1)) - 1;
			reinterpret_cast<volatile GPIO* const>(GpioAddress)->CRL &= ~(mask << maskOffset);
			reinterpret_cast<volatile GPIO* const>(GpioAddress)->CRL |= ((static_cast<uint32_t>(mode) & 0x03) << maskOffset);
			reinterpret_cast<volatile GPIO* const>(GpioAddress)->CRL |= ((static_cast<uint32_t>(oSpeed) & 0x03) << (maskOffset + 2));
		}
	}

	static inline bool get(const GpioMode mode) {
		return (reinterpret_cast<volatile GPIO* const>(GpioAddress)->IDR & GpioPinMask);
	}

	static inline void set() {
		reinterpret_cast<volatile GPIO* const>(GpioAddress)->BSRR = GpioPinMask;
		asm volatile("dsb;");
	}

	static inline void reset(){
		reinterpret_cast<volatile GPIO* const>(GpioAddress)->BRR = GpioPinMask;
		asm volatile("dsb;");
	}

	static inline void invert(){
		reinterpret_cast<volatile GPIO* const>(GpioAddress)->ODR ^= GpioPinMask;
		asm volatile("dsb;");
	}
};

#define GPIOA (reinterpret_cast<volatile GPIO* const>(GPIOA_BASE))
#define GPIOB (reinterpret_cast<volatile GPIO* const>(GPIOB_BASE))
#define GPIOC (reinterpret_cast<volatile GPIO* const>(GPIOC_BASE))

constexpr const GpioPin<GPIOA_BASE, 0> PA0;
constexpr const GpioPin<GPIOA_BASE, 1> PA1;
constexpr const GpioPin<GPIOA_BASE, 2> PA2;
constexpr const GpioPin<GPIOA_BASE, 3> PA3;
constexpr const GpioPin<GPIOA_BASE, 4> PA4;
constexpr const GpioPin<GPIOA_BASE, 5> PA5;
constexpr const GpioPin<GPIOA_BASE, 6> PA6;
constexpr const GpioPin<GPIOA_BASE, 7> PA7;
constexpr const GpioPin<GPIOA_BASE, 8> PA8;
constexpr const GpioPin<GPIOA_BASE, 9> PA9;
constexpr const GpioPin<GPIOA_BASE, 10> PA10;
constexpr const GpioPin<GPIOA_BASE, 11> PA11;
constexpr const GpioPin<GPIOA_BASE, 12> PA12;
constexpr const GpioPin<GPIOA_BASE, 13> PA13;
constexpr const GpioPin<GPIOA_BASE, 14> PA14;
constexpr const GpioPin<GPIOA_BASE, 15> PA15;

constexpr const GpioPin<GPIOB_BASE, 0> PB0;
constexpr const GpioPin<GPIOB_BASE, 1> PB1;
constexpr const GpioPin<GPIOB_BASE, 2> PB2;
constexpr const GpioPin<GPIOB_BASE, 3> PB3;
constexpr const GpioPin<GPIOB_BASE, 4> PB4;
constexpr const GpioPin<GPIOB_BASE, 5> PB5;
constexpr const GpioPin<GPIOB_BASE, 6> PB6;
constexpr const GpioPin<GPIOB_BASE, 7> PB7;
constexpr const GpioPin<GPIOB_BASE, 8> PB8;
constexpr const GpioPin<GPIOB_BASE, 9> PB9;
constexpr const GpioPin<GPIOB_BASE, 10> PB10;
constexpr const GpioPin<GPIOB_BASE, 11> PB11;
constexpr const GpioPin<GPIOB_BASE, 12> PB12;
constexpr const GpioPin<GPIOB_BASE, 13> PB13;
constexpr const GpioPin<GPIOB_BASE, 14> PB14;
constexpr const GpioPin<GPIOB_BASE, 15> PB15;

constexpr const GpioPin<GPIOC_BASE, 0> PC0;
constexpr const GpioPin<GPIOC_BASE, 1> PC1;
constexpr const GpioPin<GPIOC_BASE, 2> PC2;
constexpr const GpioPin<GPIOC_BASE, 3> PC3;
constexpr const GpioPin<GPIOC_BASE, 4> PC4;
constexpr const GpioPin<GPIOC_BASE, 5> PC5;
constexpr const GpioPin<GPIOC_BASE, 6> PC6;
constexpr const GpioPin<GPIOC_BASE, 7> PC7;
constexpr const GpioPin<GPIOC_BASE, 8> PC8;
constexpr const GpioPin<GPIOC_BASE, 9> PC9;
constexpr const GpioPin<GPIOC_BASE, 10> PC10;
constexpr const GpioPin<GPIOC_BASE, 11> PC11;
constexpr const GpioPin<GPIOC_BASE, 12> PC12;
constexpr const GpioPin<GPIOC_BASE, 13> PC13;
constexpr const GpioPin<GPIOC_BASE, 14> PC14;
constexpr const GpioPin<GPIOC_BASE, 15> PC15;

