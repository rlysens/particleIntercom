#ifndef INTERCOM_POWER_MANAGEMENT_H
#define INTERCOM_POWER_MANAGEMENT_H

class Intercom_PowerManagement {
private:
public:
	Intercom_PowerManagement();	

	void checkPowerSwitch(void);
	void powerDown(void);
};

#endif /*INTERCOM_POWER_MANAGEMENT_H*/