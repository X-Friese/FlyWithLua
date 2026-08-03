#ifndef _XPLMWeather_h_
#define _XPLMWeather_h_

/*
 * Copyright 2005-2025 Laminar Research, Sandy Barbour and Ben Supnik All
 * rights reserved.  See license.txt for usage. X-Plane SDK Version: 4.0.0
 *
 */

/***************************************************************************
 * XPLMWeather
 ***************************************************************************/
/*
 * This provides access to the X-Plane 12 enhanced weather system.
 * 
 * ALL FUNCTIONS RELATING TO UPDATING THE WEATHER ARE CURRENTLY EXPERIMENTAL,
 * FOR EVALUATION.
 * 
 * The API will be kept stable if at all possible during the evaluation
 * period. The details of how the data is processed will change over time as
 * the weather engine changes, even after the API is stabilised.
 *
 */

#include "XPLMDefs.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(XPLM400)
/***************************************************************************
 * WEATHER ACCESS
 ***************************************************************************/

/*
 * XPLMWeatherInfoWinds_t
 *
 */
typedef struct {
    /* Altitude MSL, meters.                                                      */
     float                     alt_msl;
    /* Wind speed, meters/sec.  When setting, use a negative value to signify     *
     * 'undefined'.                                                               */
     float                     speed;
    /* Direction (true)                                                           */
     float                     direction;
    /* Gust speed, meters/sec. Total speed, not increase above wind speed.        */
     float                     gust_speed;
    /* Shear arc, degrees i.e. 50% of this arc in either direction from base      */
     float                     shear;
    /* Clear-air turbulence ratio                                                 */
     float                     turbulence;
} XPLMWeatherInfoWinds_t;
/*
 * XPLMWeatherInfoClouds_t
 *
 */
typedef struct {
    /* Cloud type, float enum                                                     */
     float                     cloud_type;
    /* Coverage ratio                                                             */
     float                     coverage;
    /* Altitude MSL, meters                                                       */
     float                     alt_top;
    /* Altitude MSL, meters                                                       */
     float                     alt_base;
} XPLMWeatherInfoClouds_t;
/* The number of wind layers that are expected in the latest version of       *
 * XPLMWeatherInfo_t .                                                        */
#define XPLM_NUM_WIND_LAYERS 13
/* The number of cloud layers that are expected in the latest version of      *
 * XPLMWeatherInfo_t .                                                        */
#define XPLM_NUM_CLOUD_LAYERS 3
#if defined(XPLM420)
/* The number of temperature layers that are expected in the latest version of*
 * XPLMWeatherInfo_t .                                                        */
#define XPLM_NUM_TEMPERATURE_LAYERS 13
#endif /* XPLM420 */
#if defined(XPLM420)
/* Use this value to designate a wind layer as undefined when setting.        */
#define XPLM_WIND_UNDEFINED_LAYER -1
#endif /* XPLM420 */
#if defined(XPLM420)
/* Default radius of weather data points set using XPLMSetWeatherAtLocation   *
 * and XPLMSetWeatherAtAirport.                                               */
#define XPLM_DEFAULT_WXR_RADIUS_NM 30
#endif /* XPLM420 */
#if defined(XPLM420)
/* Default vertical limit of effect of weather data points set using          *
 * XPLMSetWeatherAtLocation and XPLMSetWeatherAtAirport.                      */
#define XPLM_DEFAULT_WXR_LIMIT_MSL_FT 10000
#endif /* XPLM420 */
/*
 * XPLMWeatherInfo_t
 * 
 * Basic weather conditions at a specific point. To specify exactly what data
 * you intend to send or receive, it is required to set the structSize
 * appropriately.
 * 
 * Version 2 data starts at "temp_layers".
 *
 */
typedef struct {
    /* The size of the struct. Required for both reading and writing.             */
     int                       structSize;
    /* Temperature at the given altitude in Celsius                               */
     float                     temperature_alt;
    /* Dewpoint at the given altitude in Celsius                                  */
     float                     dewpoint_alt;
    /* Pressure at the given altitude in Pascals. Pass 0 when setting to use      *
     * sea-level pressure instead of QNH.                                         */
     float                     pressure_alt;
    /* Precipitation rate at the given altitude. Unused when setting.             */
     float                     precip_rate_alt;
    /* Wind direction at the given altitude. Unused when setting.                 */
     float                     wind_dir_alt;
    /* Wind speed at the given altitude, meters/sec. Unused when setting.         */
     float                     wind_spd_alt;
    /* Turbulence ratio at the given altitude. Unused when setting.               */
     float                     turbulence_alt;
    /* Height of water waves in meters                                            */
     float                     wave_height;
    /* Length of water waves in meters. Unused when setting.                      */
     float                     wave_length;
    /* Direction from which water waves are coming                                */
     int                       wave_dir;
    /* Speed of wave advance in meters/sec. Unused when setting.                  */
     float                     wave_speed;
    /* Base visibility at 0 altitude, meters                                      */
     float                     visibility;
    /* Base precipitation ratio at 0 altitude                                     */
     float                     precip_rate;
    /* Climb rate due to thermals, meters/sec                                     */
     float                     thermal_climb;
    /* Pressure at sealevel in Pascals. Used when setting ONLY if pressure_alt    *
     * (i.e. QNH) is not valid.                                                   */
     float                     pressure_sl;
    /* Defined wind layers. Not all layers are always defined.                    */
     XPLMWeatherInfoWinds_t    wind_layers[XPLM_NUM_WIND_LAYERS];
    /* Defined cloud layers. Not all layers are always defined.                   */
     XPLMWeatherInfoClouds_t   cloud_layers[XPLM_NUM_CLOUD_LAYERS];
#if defined(XPLM420)
    /* Temperatures at altitude, in degrees C. Layer altitudes are the same       *
     * globally - see the 'sim/weather/region/atmosphere_alt_levels_m' dataref.   */
     float                     temp_layers[XPLM_NUM_TEMPERATURE_LAYERS];
#endif /* XPLM420 */
#if defined(XPLM420)
    /* Dewpoints at altitude, in degrees C. Layer altitudes are the same globally *
     * - see the 'sim/weather/region/atmosphere_alt_levels_m' dataref.            */
     float                     dewp_layers[XPLM_NUM_TEMPERATURE_LAYERS];
#endif /* XPLM420 */
#if defined(XPLM420)
    /* The altitude in MSL of the troposphere.                                    */
     float                     troposphere_alt;
#endif /* XPLM420 */
#if defined(XPLM420)
    /* The temperature in degrees C of the troposphere.                           */
     float                     troposphere_temp;
#endif /* XPLM420 */
#if defined(XPLM420)
    /* Age in seconds of this weather report. Age affects how strongly the report *
     * affects the resulting weather.                                             */
     float                     age;
#endif /* XPLM420 */
#if defined(XPLM420)
    /* Horizontal radius of effect of this weather report, nautical miles.        */
     float                     radius_nm;
#endif /* XPLM420 */
#if defined(XPLM420)
    /* Vertical limit of effect of this weather report, feet MSL.                 */
     float                     max_altitude_msl_ft;
#endif /* XPLM420 */
} XPLMWeatherInfo_t;
/*
 * XPLMGetMETARForAirport
 * 
 * Get the last-downloaded METAR report for an airport by ICAO code. Note that
 * the actual weather at that airport may have evolved significantly since the
 * last downloaded METAR. outMETAR must point to a char buffer of at least 150
 * characters. THIS CALL DOES NOT RETURN THE CURRENT WEATHER AT THE AIRPORT,
 * and returns an empty string if the system is not in real-weather mode.
 * 
 * This call is not intended to be used per-frame. It should be called only
 * during the pre-flight loop callback.
 *
 */
XPLM_API void       XPLMGetMETARForAirport(
                         const char *         airport_id,
                         XPLMFixedString150_t * outMETAR);
/*
 * XPLMGetWeatherAtLocation
 * 
 * Get the current weather conditions at a given location. Note that this does
 * not work world-wide, only within the surrounding region. Return 1 if
 * detailed weather (i.e. an airport-specific METAR) was found, 0 if not. In
 * both cases, the structure will contain the best data available. The passed
 * in altitude is used to sample wind data vertically.
 * 
 * This call is not intended to be used per-frame. It should be called only
 * during the pre-flight loop callback.
 *
 */
XPLM_API int        XPLMGetWeatherAtLocation(
                         double               latitude,
                         double               longitude,
                         double               altitude_m,
                         XPLMWeatherInfo_t *  out_info);
#if defined(XPLM420)
/*
 * XPLMBeginWeatherUpdate
 * 
 * Inform the simulator that you are starting a batch update of weather
 * information. If you are providing multiple weather updates, using this call
 * may improve performance by telling the simulator not to update weather
 * until you are done.
 * 
 * This call is not intended to be used per-frame.  It should be called only
 * during the pre-flight loop callback. You must call XPLMEndWeatherUpdate
 * before you return from the callback; XPLMBeginWeatherUpdate in one callback
 * and XPLMEndWeatherUpdate in a later callback, even within the same frame,
 * is not permitted.
 *
 */
XPLM_API void       XPLMBeginWeatherUpdate(void);
#endif /* XPLM420 */
#if defined(XPLM420)
/*
 * XPLMEndWeatherUpdate
 * 
 * Inform the simulator that you are ending a batch update of weather
 * information. If you have called XPLMBeginWeatherUpdate, you MUST call
 * XPLMEndWeatherUpdate before exiting your callback otherwise any accumulated
 * weather data will be discarded.
 * 
 * When using incremental mode, any changes made are applied to your existing
 * data. This makes it possible to only update a fraction of your weather data
 * at any one time. When not using incremental mode, ALL reports previously
 * passed by your plugin are erased before applying new data.
 * 
 * When using any of these 'weather set' APIs, the normal mode of operation is
 * that you are setting the weather in the near future. Currently this is
 * somewhere between one and two minutes but do not rely on this remaining the
 * same.
 * 
 * Setting future weather ensures that there is no sudden jump in weather
 * conditions when you make a change mid-cycle. In some situations, notably
 * for an initial setup, you may want to ensure that the weather is changed
 * instantly. To do this, set 'updateImmediately' as true.
 * 
 * isIncremental     : If true, append or modify existing records created by
 * your plugin. If false, clear any existing records. updateImmediately : If
 * true, immediately reset and recalculate the weather. If false, your new
 * data will be used when the weather next recalculates.
 * 
 * This call is not intended to be used per-frame. It should be called only
 * during the pre-flight loop callback.
 *
 */
XPLM_API void       XPLMEndWeatherUpdate(
                         int                  isIncremental,
                         int                  updateImmediately);
#endif /* XPLM420 */
#if defined(XPLM420)
/*
 * XPLMSetWeatherAtLocation
 * 
 * Set the current weather conditions at a given location on the ground and
 * above it.. Please see the notes on individual fields in 
 * XPLMSetWeatherAtAirport, and notes on timing in XPLMEndWeatherUpdate.
 * 
 * The ground altitude passed into this function call does not set the area of
 * influence of this weather vertically; the weather takes effect from 0 MSL
 * ground up to the passed in max_altitude_msl_ft  The ground altitude passed
 * in is the elevation of the reporting station to calibrate QNH.
 * 
 * This call is not intended to be used per-frame. It should be called only
 * during the pre-flight loop callback.
 *
 */
XPLM_API void       XPLMSetWeatherAtLocation(
                         double               latitude,
                         double               longitude,
                         double               ground_altitude_msl,
                         XPLMWeatherInfo_t *  in_info);
#endif /* XPLM420 */
#if defined(XPLM420)
/*
 * XPLMEraseWeatherAtLocation
 * 
 * Erase weather conditions set by your plugin at a given location. You must
 * give exactly the same coordinates that you used to create a weather record
 * at this point. It does NOT mean 'create clear weather at this location'.
 * 
 * This call is not intended to be used per-frame. It should be called only
 * during the pre-flight loop callback.
 *
 */
XPLM_API void       XPLMEraseWeatherAtLocation(
                         double               latitude,
                         double               longitude);
#endif /* XPLM420 */
#if defined(XPLM420)
/*
 * XPLMSetWeatherAtAirport
 * 
 * Set the current weather conditions at a given airport. Unlike
 * XPLMSetWeatherAtLocation, this call will replace any existing weather
 * records for that airport from other sources (i.e. downloaded METARs)
 * instead of being used as just another weather sample.
 * 
 * Some notes on individual fields:
 *   - pressure_alt should be QNH as reported by a station at the specified
 *     airport, or 0 if you are passing sealevel pressure in 'pressure_sl'
 *     instead.
 *   - pressure_sl is ignored if pressure_alt is given.
 *   - wind_dir_alt, wind_spd_alt, turbulence_alt, wave_speed, wave_length are
 *     derived from other data and are UNUSED when setting weather.
 *   - Temperatures can be given either as a single temperature at the ground
 *     altitude (temperature_alt) OR, if the struct is V2 or higher, as an
 *     array of temperatures aloft (temp_layers). If you pass a value for
 *     temperature_alt higher than -273.15 (absolute zero), that will be used
 *     with the altitude value to calculate an offset from ISA temperature at
 *     all altitudes. Any layer in temp_layers for which you set the
 *     temperature higher than -273.15 (absolute zero) will use that
 *     temperature and all others will use the existing value for the
 *     location, or the calculated values from temperature_alt if you also
 *     passed that. It is advised to use a lower value than exactly -273.15 to
 *     avoid floating-point precision errors. These calculated temperatures
 *     during a read are also affected by the troposphere altitude and
 *     temperature, and the vertical radius of effect. Do not expect to get
 *     the exact values you set.
 *   - The same rules apply to dewpoint temperatures; either a single value at
 *     ground level in 'dewpoint_alt', or per-layer values in 'dewp_layers'.
 *   - The troposphere altitude and temperature will be derived from existing
 *     data if you pass 0 or lower for troposphere_alt. Both altitude and
 *     temperature may be clamped to internally-defined ranges.
 *   - When setting both temperature and dewpoint from a single value
 *     (temperature_alt/dewpoint_alt), the rest of the atmosphere will be
 *     graded to fit between the given values and the troposphere.
 * 
 * This call is not intended to be used per-frame. It should be called only
 * during the pre-flight loop callback.
 *
 */
XPLM_API void       XPLMSetWeatherAtAirport(
                         const char *         airport_id,
                         XPLMWeatherInfo_t *  in_info);
#endif /* XPLM420 */
#if defined(XPLM420)
/*
 * XPLMEraseWeatherAtAirport
 * 
 * Erase the current weather conditions set by your plugin at a given airport,
 * allowing records from other sources to be used. It does NOT mean 'create
 * clear weather at this airport'.
 * 
 * This call is not intended to be used per-frame. It should be called only
 * during the pre-flight loop callback.
 *
 */
XPLM_API void       XPLMEraseWeatherAtAirport(
                         const char *         airport_id);
#endif /* XPLM420 */
#endif /* XPLM400 */
#ifdef __cplusplus
}
#endif

#endif
