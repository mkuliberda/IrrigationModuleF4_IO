/*
 * Plants.h
 *
 *  Created on: 28.11.2019
 *      Author: Mati
 */

#ifndef PLANTS_H_
#define PLANTS_H_

#include "stm32f4xx_hal.h"
#include "freertos_memory.h"
#include <string>
#include <vector>
#include <utilities.h>

#ifndef PLANT_NAME_LEN
	#define PLANT_NAME_LEN 20
#endif

enum plant_type_t: uint8_t {
	plant_only,
	plant_with_dma_moisture_sensor
};

struct PlantInfo_s{
	uint8_t			id;
	plant_type_t	p_type;
	bool			rain_exposed;
	float			soil_moisture_percent;
	char			name[PLANT_NAME_LEN];
};

//Decorator design pattern used for plants
class PlantInterface {
public:
	//PlantInterface(const PlantInterface&) = default;
	//PlantInterface& operator= (const PlantInterface&) = default;
	//PlantInterface(PlantInterface &&) = default;
	//PlantInterface& operator= (PlantInterface &&) = default;

	virtual ~PlantInterface() =default; 
	virtual float getMoisturePercent() = 0;
	virtual bool setMoisturePercent(const float &_moisture) = 0;
	virtual std::string getName() = 0;
	virtual uint8_t getId() = 0;
	virtual void updateRaw(const uint32_t &_raw_measurement) = 0;
	virtual void setPlantType(const plant_type_t& _p_type=plant_only) = 0;
	virtual plant_type_t getPlantType() = 0;
	virtual void setRainExposition(const bool& _rain_exposed) = 0;
	virtual bool isRainExposed() = 0;
};



class Plant : public PlantInterface {
private:

	plant_type_t									p_type{plant_type_t::plant_only};
	std::string										name;
	float											soil_moisture_percent{-1000};
	uint8_t											id;
	bool											rain_exposed;

public:
	Plant(const std::string_view& _name, const uint8_t& _id, const bool& _rain_exposed) :
		name(_name),
		id(_id),
		rain_exposed(_rain_exposed)
	{
		name.shrink_to_fit();
		p_type = plant_only;
	}

	Plant(const std::string_view&& _name, const uint8_t&& _id, const bool&& _rain_exposed) :
		name(std::move(_name)),
		id(std::move(_id)),
		rain_exposed(std::move(_rain_exposed))
	{
		name.shrink_to_fit();
		p_type = plant_only;
	}

	~Plant() =default;

	//To avoid runtime errors, delete copy constructor and copy assignment operator. If sth's wrong, compile time error will fire.
	Plant(Plant const &) = delete;				
	Plant& operator=(Plant const&) = delete;


	/*virtual*/
	float											getMoisturePercent(void);
	std::string										getName(void);
	uint8_t											getId(void);
	bool											setMoisturePercent(const float& _moisture);
	void											updateRaw(const uint32_t& _raw_measurement);
	void											setPlantType(const plant_type_t& _p_type = plant_only);
	plant_type_t									getPlantType(void);
	void											setRainExposition(const bool& _rain_exposed);
	bool											isRainExposed();
};

class PlantWithSensor : public PlantInterface {
private:
	PlantInterface									*m_wrappee;

public:
	PlantWithSensor(PlantInterface *inner) {
		m_wrappee = inner;
	}
	~PlantWithSensor() {
		delete m_wrappee;
	}

	//To avoid runtime errors, delete copy constructor and copy assignment operator. If sth's wrong, compile time error will fire.
	PlantWithSensor(PlantWithSensor const &) = delete;
	PlantWithSensor& operator=(PlantWithSensor const&) = delete;

	float											getMoisturePercent();
	bool											setMoisturePercent(const float& _moisture);
	std::string										getName();
	uint8_t											getId();
	void											updateRaw(const uint32_t &_raw_measurement);
	void											setPlantType(const plant_type_t& _p_type = plant_only);
	plant_type_t									getPlantType(void);
	void											setRainExposition(const bool& _rain_exposed);
	bool											isRainExposed();
};

class PlantWithDMAMoistureSensor : public PlantWithSensor {
private:
	float											ref_voltage;
	uint32_t										quantization_levels;
	float											soil_moisture_volts;
	float											soil_moisture_raw;

public:
	PlantWithDMAMoistureSensor(PlantInterface *core, const float& _ref_voltage = 3.3, const uint32_t& _quantization_levels = 4095) :
		PlantWithSensor(core),
		ref_voltage(_ref_voltage),
		quantization_levels(_quantization_levels){
		PlantWithSensor::setPlantType(plant_type_t::plant_with_dma_moisture_sensor);
		//setPlantType(); //TODO: "calling virtual function in constructor is dangerous"
	}

	PlantWithDMAMoistureSensor(PlantInterface *core, const float&& _ref_voltage = 3.3, const uint32_t&& _quantization_levels = 4095) :
		PlantWithSensor(core),
		ref_voltage(std::move(_ref_voltage)),
		quantization_levels(std::move(_quantization_levels)) {
		PlantWithSensor::setPlantType(plant_type_t::plant_with_dma_moisture_sensor);
		//setPlantType(); //TODO: "calling virtual function in constructor is dangerous"
	}

	~PlantWithDMAMoistureSensor() =default;

	//To avoid runtime errors, delete copy constructor and copy assignment operator. If sth's wrong, compile time error will fire.
	PlantWithDMAMoistureSensor(PlantWithDMAMoistureSensor const &) = delete;
	PlantWithDMAMoistureSensor& operator=(PlantWithDMAMoistureSensor const&) = delete;

	/*virtual*/
	float											getMoisturePercent();
	float											getMoistureVolts();
	float											getMoistureRaw();
	std::string										getName();
	uint8_t											getId();
	void											updateRaw(const uint32_t& _raw_measurement);
	void											setPlantType(const plant_type_t& _p_type = plant_only);
	plant_type_t									getPlantType();
	void											setRainExposition(const bool& _rain_exposed);
	bool											isRainExposed();
};

#endif /* PLANTS_H_ */
