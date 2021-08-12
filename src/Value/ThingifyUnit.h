#ifndef _THINGIFY_UNIT
#define _THINGIFY_UNIT

enum class ThingifyUnit
{	
	None,
	Percent,
	Permille,
	/* Temperature */
	Celsius,
	Fahrenheit,
	Kelvin,

	/* Distance */
	Millimetre,
	Centimetre,
	Meter,
	Mile,


	/* Data size */
	Byte,
	Kilobyte,
	Megabyte,
	Gigabyte,
	Terabyte,

	/* Frequency */

	Hertz,
	Kilohertz,
	Megahertz,
	Gigahertz,

	/* Time */
	Minute,
	Hour,
	Second,
	Day,
	Year,

	/* Weight */
	Kilogram,
	Tone,
	Pound,
	/* Preseure */
	Atmosphere,
	Bar,
	Millibar,
	Pascal,
	Hectopascal,
	Kilopascal,
	Gigapascal,
	Megapascal,
	Psi,
	Torr,
	InHg,
	CmHg,

	/* Electricity */
	Ampere,
	Millivolt,
	Volt,
	Kilovolt,
	Megavolt,

	/* Light */
	Lx,

	/* Particles*/
	Ppm,
	Ppb,
	UgPerM3,

	/* signal strength */
	Rssi,
	dBm,

	/* Power */
	mW,
	W,
	kW,
	MW,
	GW,
	kWh,
	MWh,
	GWh

};

#endif