/*
 * ramn_sensors.c
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2025 TOYOTA MOTOR CORPORATION.
 * ALL RIGHTS RESERVED.</center></h2>
 *
 * This software component is licensed by TOYOTA MOTOR CORPORATION under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */

#include "ramn_sensors.h"

// Public --------------------------------------------

// Array directly accessed by DMA
#if defined(EXPANSION_CHASSIS) || defined(EXPANSION_POWERTRAIN) || defined(EXPANSION_BODY)
uint16_t RAMN_SENSORS_ADCValues[NUMBER_OF_ADC] = {0,0,0};
#endif

// Private --------------------------------------------

// Variable to keep the tick of the last update.
static uint32_t lastUpdateTick; //TODO: remove and replace by better debounce

#if defined(EXPANSION_CHASSIS)

// Look up table to convert logarithmic potentiometer to linear potentiometer
// By default not applied to BRAKE and ACCEL, to mimic real pedals.
#if defined(CHASSIS_LOGARITHMIC_POTENTIOMETER)
static const uint16_t loglookupt[] = {
		0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xffe, 0xffe, 0xffe, 0xffd, 0xffd, 0xffd, 0xffc, 0xffc, 0xffc, 0xffb, 0xffb, 0xffb, 0xffa, 0xffa, 0xff9, 0xff9, 0xff9, 0xff8, 0xff8, 0xff8, 0xff7, 0xff7, 0xff7, 0xff6, 0xff6, 0xff6, 0xff5, 0xff5, 0xff5, 0xff4, 0xff4, 0xff4, 0xff3, 0xff3, 0xff3, 0xff2, 0xff2, 0xff2, 0xff1, 0xff1, 0xff1, 0xff0, 0xff0, 0xff0, 0xfef, 0xfef, 0xfee, 0xfee, 0xfee, 0xfed, 0xfed, 0xfed, 0xfec, 0xfec, 0xfec, 0xfeb, 0xfeb, 0xfeb, 0xfea, 0xfea, 0xfea, 0xfe9, 0xfe9, 0xfe9, 0xfe8, 0xfe8, 0xfe8, 0xfe7, 0xfe7, 0xfe7, 0xfe6, 0xfe6, 0xfe6, 0xfe5, 0xfe5, 0xfe4, 0xfe4, 0xfe4, 0xfe3, 0xfe3, 0xfe3, 0xfe2, 0xfe2, 0xfe2, 0xfe1, 0xfe1, 0xfe1, 0xfe0, 0xfe0, 0xfe0, 0xfdf, 0xfdf, 0xfdf, 0xfde, 0xfde, 0xfde, 0xfdd, 0xfdd, 0xfdc, 0xfdc, 0xfdc, 0xfdb, 0xfdb, 0xfdb, 0xfda, 0xfda, 0xfda, 0xfd9, 0xfd9, 0xfd9, 0xfd8, 0xfd8, 0xfd8, 0xfd7, 0xfd7, 0xfd7, 0xfd6, 0xfd6, 0xfd5, 0xfd5, 0xfd5, 0xfd4, 0xfd4, 0xfd4, 0xfd3, 0xfd3, 0xfd3, 0xfd2, 0xfd2, 0xfd2, 0xfd1, 0xfd1, 0xfd1, 0xfd0, 0xfd0, 0xfcf, 0xfcf, 0xfcf, 0xfce, 0xfce, 0xfce, 0xfcd, 0xfcd, 0xfcd, 0xfcc, 0xfcc, 0xfcc, 0xfcb, 0xfcb, 0xfcb, 0xfca, 0xfca, 0xfc9, 0xfc9, 0xfc9, 0xfc8, 0xfc8, 0xfc8, 0xfc7, 0xfc7, 0xfc7, 0xfc6, 0xfc6, 0xfc6, 0xfc5, 0xfc5, 0xfc5, 0xfc4, 0xfc4, 0xfc3, 0xfc3, 0xfc3, 0xfc2, 0xfc2, 0xfc2, 0xfc1, 0xfc1, 0xfc1, 0xfc0, 0xfc0, 0xfc0, 0xfbf, 0xfbf, 0xfbe, 0xfbe, 0xfbe, 0xfbd, 0xfbd, 0xfbd, 0xfbc, 0xfbc, 0xfbc, 0xfbb, 0xfbb, 0xfbb, 0xfba, 0xfba, 0xfb9, 0xfb9, 0xfb9, 0xfb8, 0xfb8, 0xfb8, 0xfb7, 0xfb7, 0xfb7, 0xfb6, 0xfb6, 0xfb6, 0xfb5, 0xfb5, 0xfb4, 0xfb4, 0xfb4, 0xfb3, 0xfb3, 0xfb3, 0xfb2, 0xfb2, 0xfb2, 0xfb1, 0xfb1, 0xfb0, 0xfb0, 0xfb0, 0xfaf, 0xfaf, 0xfaf, 0xfae, 0xfae, 0xfae, 0xfad, 0xfad, 0xfad, 0xfac, 0xfac, 0xfab, 0xfab, 0xfab, 0xfaa, 0xfaa, 0xfaa, 0xfa9, 0xfa9, 0xfa9, 0xfa8, 0xfa8, 0xfa7, 0xfa7, 0xfa7, 0xfa6, 0xfa6, 0xfa6, 0xfa5, 0xfa5, 0xfa5, 0xfa4, 0xfa4, 0xfa3, 0xfa3, 0xfa3, 0xfa2, 0xfa2, 0xfa2, 0xfa1, 0xfa1, 0xfa1, 0xfa0, 0xfa0, 0xf9f, 0xf9f, 0xf9f, 0xf9e, 0xf9e, 0xf9e, 0xf9d, 0xf9d, 0xf9d, 0xf9c, 0xf9c, 0xf9b, 0xf9b, 0xf9b, 0xf9a, 0xf9a, 0xf9a, 0xf99, 0xf99, 0xf99, 0xf98, 0xf98, 0xf97, 0xf97, 0xf97, 0xf96, 0xf96, 0xf96, 0xf95, 0xf95, 0xf95, 0xf94, 0xf94, 0xf93, 0xf93, 0xf93, 0xf92, 0xf92, 0xf92, 0xf91, 0xf91, 0xf90, 0xf90, 0xf90, 0xf8f, 0xf8f, 0xf8f, 0xf8e, 0xf8e, 0xf8e, 0xf8d, 0xf8d, 0xf8c, 0xf8c, 0xf8c, 0xf8b, 0xf8b, 0xf8b, 0xf8a, 0xf8a, 0xf89, 0xf89, 0xf89, 0xf88, 0xf88, 0xf88, 0xf87, 0xf87, 0xf86, 0xf86, 0xf86, 0xf85, 0xf85, 0xf85, 0xf84, 0xf84, 0xf84, 0xf83, 0xf83, 0xf82, 0xf82, 0xf82, 0xf81, 0xf81, 0xf81, 0xf80, 0xf80, 0xf7f, 0xf7f, 0xf7f, 0xf7e, 0xf7e, 0xf7e, 0xf7d, 0xf7d, 0xf7c, 0xf7c, 0xf7c, 0xf7b, 0xf7b, 0xf7b, 0xf7a, 0xf7a, 0xf79, 0xf79, 0xf79, 0xf78, 0xf78, 0xf78, 0xf77, 0xf77, 0xf76, 0xf76, 0xf76, 0xf75, 0xf75, 0xf75, 0xf74, 0xf74, 0xf73, 0xf73, 0xf73, 0xf72, 0xf72, 0xf72, 0xf71, 0xf71, 0xf70, 0xf70, 0xf70, 0xf6f, 0xf6f, 0xf6f, 0xf6e, 0xf6e, 0xf6d, 0xf6d, 0xf6d, 0xf6c, 0xf6c, 0xf6c, 0xf6b, 0xf6b, 0xf6a, 0xf6a, 0xf6a, 0xf69, 0xf69, 0xf69, 0xf68, 0xf68, 0xf67, 0xf67, 0xf67, 0xf66, 0xf66, 0xf65, 0xf65, 0xf65, 0xf64, 0xf64, 0xf64, 0xf63, 0xf63, 0xf62, 0xf62, 0xf62, 0xf61, 0xf61, 0xf61, 0xf60, 0xf60, 0xf5f, 0xf5f, 0xf5f, 0xf5e, 0xf5e, 0xf5e, 0xf5d, 0xf5d, 0xf5c, 0xf5c, 0xf5c, 0xf5b, 0xf5b, 0xf5a, 0xf5a, 0xf5a, 0xf59, 0xf59, 0xf59, 0xf58, 0xf58, 0xf57, 0xf57, 0xf57, 0xf56, 0xf56, 0xf55, 0xf55, 0xf55, 0xf54, 0xf54, 0xf54, 0xf53, 0xf53, 0xf52, 0xf52, 0xf52, 0xf51, 0xf51, 0xf50, 0xf50, 0xf50, 0xf4f, 0xf4f, 0xf4f, 0xf4e, 0xf4e, 0xf4d, 0xf4d, 0xf4d, 0xf4c, 0xf4c, 0xf4b, 0xf4b, 0xf4b, 0xf4a, 0xf4a, 0xf4a, 0xf49, 0xf49, 0xf48, 0xf48, 0xf48, 0xf47, 0xf47, 0xf46, 0xf46, 0xf46, 0xf45, 0xf45, 0xf44, 0xf44, 0xf44, 0xf43, 0xf43, 0xf43, 0xf42, 0xf42, 0xf41, 0xf41, 0xf41, 0xf40, 0xf40, 0xf3f, 0xf3f, 0xf3f, 0xf3e, 0xf3e, 0xf3d, 0xf3d, 0xf3d, 0xf3c, 0xf3c, 0xf3c, 0xf3b, 0xf3b, 0xf3a, 0xf3a, 0xf3a, 0xf39, 0xf39, 0xf38, 0xf38, 0xf38, 0xf37, 0xf37, 0xf36, 0xf36, 0xf36, 0xf35, 0xf35, 0xf34, 0xf34, 0xf34, 0xf33, 0xf33, 0xf33, 0xf32, 0xf32, 0xf31, 0xf31, 0xf31, 0xf30, 0xf30, 0xf2f, 0xf2f, 0xf2f, 0xf2e, 0xf2e, 0xf2d, 0xf2d, 0xf2d, 0xf2c, 0xf2c, 0xf2b, 0xf2b, 0xf2b, 0xf2a, 0xf2a, 0xf29, 0xf29, 0xf29, 0xf28, 0xf28, 0xf27, 0xf27, 0xf27, 0xf26, 0xf26, 0xf26, 0xf25, 0xf25, 0xf24, 0xf24, 0xf24, 0xf23, 0xf23, 0xf22, 0xf22, 0xf22, 0xf21, 0xf21, 0xf20, 0xf20, 0xf20, 0xf1f, 0xf1f, 0xf1e, 0xf1e, 0xf1e, 0xf1d, 0xf1d, 0xf1c, 0xf1c, 0xf1c, 0xf1b, 0xf1b, 0xf1a, 0xf1a, 0xf1a, 0xf19, 0xf19, 0xf18, 0xf18, 0xf18, 0xf17, 0xf17, 0xf16, 0xf16, 0xf16, 0xf15, 0xf15, 0xf14, 0xf14, 0xf14, 0xf13, 0xf13, 0xf12, 0xf12, 0xf12, 0xf11, 0xf11, 0xf10, 0xf10, 0xf10, 0xf0f, 0xf0f, 0xf0e, 0xf0e, 0xf0e, 0xf0d, 0xf0d, 0xf0c, 0xf0c, 0xf0b, 0xf0b, 0xf0b, 0xf0a, 0xf0a, 0xf09, 0xf09, 0xf09, 0xf08, 0xf08, 0xf07, 0xf07, 0xf07, 0xf06, 0xf06, 0xf05, 0xf05, 0xf05, 0xf04, 0xf04, 0xf03, 0xf03, 0xf03, 0xf02, 0xf02, 0xf01, 0xf01, 0xf01, 0xf00, 0xf00, 0xeff, 0xeff, 0xeff, 0xefe, 0xefe, 0xefd, 0xefd, 0xefc, 0xefc, 0xefc, 0xefb, 0xefb, 0xefa, 0xefa, 0xefa, 0xef9, 0xef9, 0xef8, 0xef8, 0xef8, 0xef7, 0xef7, 0xef6, 0xef6, 0xef6, 0xef5, 0xef5, 0xef4, 0xef4, 0xef3, 0xef3, 0xef3, 0xef2, 0xef2, 0xef1, 0xef1, 0xef1, 0xef0, 0xef0, 0xeef, 0xeef, 0xeef, 0xeee, 0xeee, 0xeed, 0xeed, 0xeec, 0xeec, 0xeec, 0xeeb, 0xeeb, 0xeea, 0xeea, 0xeea, 0xee9, 0xee9, 0xee8, 0xee8, 0xee8, 0xee7, 0xee7, 0xee6, 0xee6, 0xee5, 0xee5, 0xee5, 0xee4, 0xee4, 0xee3, 0xee3, 0xee3, 0xee2, 0xee2, 0xee1, 0xee1, 0xee0, 0xee0, 0xee0, 0xedf, 0xedf, 0xede, 0xede, 0xede, 0xedd, 0xedd, 0xedc, 0xedc, 0xedb, 0xedb, 0xedb, 0xeda, 0xeda, 0xed9, 0xed9, 0xed9, 0xed8, 0xed8, 0xed7, 0xed7, 0xed6, 0xed6, 0xed6, 0xed5, 0xed5, 0xed4, 0xed4, 0xed4, 0xed3, 0xed3, 0xed2, 0xed2, 0xed1, 0xed1, 0xed1, 0xed0, 0xed0, 0xecf, 0xecf, 0xece, 0xece, 0xece, 0xecd, 0xecd, 0xecc, 0xecc, 0xecc, 0xecb, 0xecb, 0xeca, 0xeca, 0xec9, 0xec9, 0xec9, 0xec8, 0xec8, 0xec7, 0xec7, 0xec6, 0xec6, 0xec6, 0xec5, 0xec5, 0xec4, 0xec4, 0xec3, 0xec3, 0xec3, 0xec2, 0xec2, 0xec1, 0xec1, 0xec0, 0xec0, 0xec0, 0xebf, 0xebf, 0xebe, 0xebe, 0xebe, 0xebd, 0xebd, 0xebc, 0xebc, 0xebb, 0xebb, 0xebb, 0xeba, 0xeba, 0xeb9, 0xeb9, 0xeb8, 0xeb8, 0xeb8, 0xeb7, 0xeb7, 0xeb6, 0xeb6, 0xeb5, 0xeb5, 0xeb5, 0xeb4, 0xeb4, 0xeb3, 0xeb3, 0xeb2, 0xeb2, 0xeb2, 0xeb1, 0xeb1, 0xeb0, 0xeb0, 0xeaf, 0xeaf, 0xeaf, 0xeae, 0xeae, 0xead, 0xead, 0xeac, 0xeac, 0xeab, 0xeab, 0xeab, 0xeaa, 0xeaa, 0xea9, 0xea9, 0xea8, 0xea8, 0xea8, 0xea7, 0xea7, 0xea6, 0xea6, 0xea5, 0xea5, 0xea5, 0xea4, 0xea4, 0xea3, 0xea3, 0xea2, 0xea2, 0xea2, 0xea1, 0xea1, 0xea0, 0xea0, 0xe9f, 0xe9f, 0xe9e, 0xe9e, 0xe9e, 0xe9d, 0xe9d, 0xe9c, 0xe9c, 0xe9b, 0xe9b, 0xe9b, 0xe9a, 0xe9a, 0xe99, 0xe99, 0xe98, 0xe98, 0xe97, 0xe97, 0xe97, 0xe96, 0xe96, 0xe95, 0xe95, 0xe94, 0xe94, 0xe94, 0xe93, 0xe93, 0xe92, 0xe92, 0xe91, 0xe91, 0xe90, 0xe90, 0xe90, 0xe8f, 0xe8f, 0xe8e, 0xe8e, 0xe8d, 0xe8d, 0xe8d, 0xe8c, 0xe8c, 0xe8b, 0xe8b, 0xe8a, 0xe8a, 0xe89, 0xe89, 0xe89, 0xe88, 0xe88, 0xe87, 0xe87, 0xe86, 0xe86, 0xe85, 0xe85, 0xe85, 0xe84, 0xe84, 0xe83, 0xe83, 0xe82, 0xe82, 0xe81, 0xe81, 0xe81, 0xe80, 0xe80, 0xe7f, 0xe7f, 0xe7e, 0xe7e, 0xe7d, 0xe7d, 0xe7d, 0xe7c, 0xe7c, 0xe7b, 0xe7b, 0xe7a, 0xe7a, 0xe79, 0xe79, 0xe79, 0xe78, 0xe78, 0xe77, 0xe77, 0xe76, 0xe76, 0xe75, 0xe75, 0xe74, 0xe74, 0xe74, 0xe73, 0xe73, 0xe72, 0xe72, 0xe71, 0xe71, 0xe70, 0xe70, 0xe70, 0xe6f, 0xe6f, 0xe6e, 0xe6e, 0xe6d, 0xe6d, 0xe6c, 0xe6c, 0xe6b, 0xe6b, 0xe6b, 0xe6a, 0xe6a, 0xe69, 0xe69, 0xe68, 0xe68, 0xe67, 0xe67, 0xe67, 0xe66, 0xe66, 0xe65, 0xe65, 0xe64, 0xe64, 0xe63, 0xe63, 0xe62, 0xe62, 0xe62, 0xe61, 0xe61, 0xe60, 0xe60, 0xe5f, 0xe5f, 0xe5e, 0xe5e, 0xe5d, 0xe5d, 0xe5d, 0xe5c, 0xe5c, 0xe5b, 0xe5b, 0xe5a, 0xe5a, 0xe59, 0xe59, 0xe58, 0xe58, 0xe57, 0xe57, 0xe57, 0xe56, 0xe56, 0xe55, 0xe55, 0xe54, 0xe54, 0xe53, 0xe53, 0xe52, 0xe52, 0xe52, 0xe51, 0xe51, 0xe50, 0xe50, 0xe4f, 0xe4f, 0xe4e, 0xe4e, 0xe4d, 0xe4d, 0xe4c, 0xe4c, 0xe4c, 0xe4b, 0xe4b, 0xe4a, 0xe4a, 0xe49, 0xe49, 0xe48, 0xe48, 0xe47, 0xe47, 0xe46, 0xe46, 0xe46, 0xe45, 0xe45, 0xe44, 0xe44, 0xe43, 0xe43, 0xe42, 0xe42, 0xe41, 0xe41, 0xe40, 0xe40, 0xe3f, 0xe3f, 0xe3f, 0xe3e, 0xe3e, 0xe3d, 0xe3d, 0xe3c, 0xe3c, 0xe3b, 0xe3b, 0xe3a, 0xe3a, 0xe39, 0xe39, 0xe38, 0xe38, 0xe38, 0xe37, 0xe37, 0xe36, 0xe36, 0xe35, 0xe35, 0xe34, 0xe34, 0xe33, 0xe33, 0xe32, 0xe32, 0xe31, 0xe31, 0xe31, 0xe30, 0xe30, 0xe2f, 0xe2f, 0xe2e, 0xe2e, 0xe2d, 0xe2d, 0xe2c, 0xe2c, 0xe2b, 0xe2b, 0xe2a, 0xe2a, 0xe29, 0xe29, 0xe29, 0xe28, 0xe28, 0xe27, 0xe27, 0xe26, 0xe26, 0xe25, 0xe25, 0xe24, 0xe24, 0xe23, 0xe23, 0xe22, 0xe22, 0xe21, 0xe21, 0xe20, 0xe20, 0xe1f, 0xe1f, 0xe1f, 0xe1e, 0xe1e, 0xe1d, 0xe1d, 0xe1c, 0xe1c, 0xe1b, 0xe1b, 0xe1a, 0xe1a, 0xe19, 0xe19, 0xe18, 0xe18, 0xe17, 0xe17, 0xe16, 0xe16, 0xe15, 0xe15, 0xe15, 0xe14, 0xe14, 0xe13, 0xe13, 0xe12, 0xe12, 0xe11, 0xe11, 0xe10, 0xe10, 0xe0f, 0xe0f, 0xe0e, 0xe0e, 0xe0d, 0xe0d, 0xe0c, 0xe0c, 0xe0b, 0xe0b, 0xe0a, 0xe0a, 0xe09, 0xe09, 0xe08, 0xe08, 0xe07, 0xe07, 0xe07, 0xe06, 0xe06, 0xe05, 0xe05, 0xe04, 0xe04, 0xe03, 0xe03, 0xe02, 0xe02, 0xe01, 0xe01, 0xe00, 0xe00, 0xdff, 0xdff, 0xdfe, 0xdfe, 0xdfd, 0xdfd, 0xdfc, 0xdfc, 0xdfb, 0xdfb, 0xdfa, 0xdfa, 0xdf9, 0xdf9, 0xdf8, 0xdf8, 0xdf7, 0xdf7, 0xdf6, 0xdf6, 0xdf5, 0xdf5, 0xdf4, 0xdf4, 0xdf4, 0xdf3, 0xdf3, 0xdf2, 0xdf2, 0xdf1, 0xdf1, 0xdf0, 0xdf0, 0xdef, 0xdef, 0xdee, 0xdee, 0xded, 0xded, 0xdec, 0xdec, 0xdeb, 0xdeb, 0xdea, 0xdea, 0xde9, 0xde9, 0xde8, 0xde8, 0xde7, 0xde7, 0xde6, 0xde6, 0xde5, 0xde5, 0xde4, 0xde4, 0xde3, 0xde3, 0xde2, 0xde2, 0xde1, 0xde1, 0xde0, 0xde0, 0xddf, 0xddf, 0xdde, 0xdde, 0xddd, 0xddd, 0xddc, 0xddc, 0xddb, 0xddb, 0xdda, 0xdda, 0xdd9, 0xdd9, 0xdd8, 0xdd8, 0xdd7, 0xdd7, 0xdd6, 0xdd6, 0xdd5, 0xdd5, 0xdd4, 0xdd4, 0xdd3, 0xdd3, 0xdd2, 0xdd2, 0xdd1, 0xdd1, 0xdd0, 0xdd0, 0xdcf, 0xdcf, 0xdce, 0xdce, 0xdcd, 0xdcd, 0xdcc, 0xdcc, 0xdcb, 0xdcb, 0xdca, 0xdca, 0xdc9, 0xdc9, 0xdc8, 0xdc8, 0xdc7, 0xdc7, 0xdc6, 0xdc6, 0xdc5, 0xdc5, 0xdc4, 0xdc4, 0xdc3, 0xdc3, 0xdc2, 0xdc2, 0xdc1, 0xdc1, 0xdc0, 0xdc0, 0xdbf, 0xdbf, 0xdbe, 0xdbe, 0xdbd, 0xdbd, 0xdbc, 0xdbb, 0xdbb, 0xdba, 0xdba, 0xdb9, 0xdb9, 0xdb8, 0xdb8, 0xdb7, 0xdb7, 0xdb6, 0xdb6, 0xdb5, 0xdb5, 0xdb4, 0xdb4, 0xdb3, 0xdb3, 0xdb2, 0xdb2, 0xdb1, 0xdb1, 0xdb0, 0xdb0, 0xdaf, 0xdaf, 0xdae, 0xdae, 0xdad, 0xdad, 0xdac, 0xdac, 0xdab, 0xdab, 0xdaa, 0xdaa, 0xda9, 0xda8, 0xda8, 0xda7, 0xda7, 0xda6, 0xda6, 0xda5, 0xda5, 0xda4, 0xda4, 0xda3, 0xda3, 0xda2, 0xda2, 0xda1, 0xda1, 0xda0, 0xda0, 0xd9f, 0xd9f, 0xd9e, 0xd9e, 0xd9d, 0xd9d, 0xd9c, 0xd9c, 0xd9b, 0xd9a, 0xd9a, 0xd99, 0xd99, 0xd98, 0xd98, 0xd97, 0xd97, 0xd96, 0xd96, 0xd95, 0xd95, 0xd94, 0xd94, 0xd93, 0xd93, 0xd92, 0xd92, 0xd91, 0xd91, 0xd90, 0xd8f, 0xd8f, 0xd8e, 0xd8e, 0xd8d, 0xd8d, 0xd8c, 0xd8c, 0xd8b, 0xd8b, 0xd8a, 0xd8a, 0xd89, 0xd89, 0xd88, 0xd88, 0xd87, 0xd86, 0xd86, 0xd85, 0xd85, 0xd84, 0xd84, 0xd83, 0xd83, 0xd82, 0xd82, 0xd81, 0xd81, 0xd80, 0xd80, 0xd7f, 0xd7f, 0xd7e, 0xd7d, 0xd7d, 0xd7c, 0xd7c, 0xd7b, 0xd7b, 0xd7a, 0xd7a, 0xd79, 0xd79, 0xd78, 0xd78, 0xd77, 0xd77, 0xd76, 0xd75, 0xd75, 0xd74, 0xd74, 0xd73, 0xd73, 0xd72, 0xd72, 0xd71, 0xd71, 0xd70, 0xd70, 0xd6f, 0xd6e, 0xd6e, 0xd6d, 0xd6d, 0xd6c, 0xd6c, 0xd6b, 0xd6b, 0xd6a, 0xd6a, 0xd69, 0xd69, 0xd68, 0xd67, 0xd67, 0xd66, 0xd66, 0xd65, 0xd65, 0xd64, 0xd64, 0xd63, 0xd63, 0xd62, 0xd61, 0xd61, 0xd60, 0xd60, 0xd5f, 0xd5f, 0xd5e, 0xd5e, 0xd5d, 0xd5d, 0xd5c, 0xd5c, 0xd5b, 0xd5a, 0xd5a, 0xd59, 0xd59, 0xd58, 0xd58, 0xd57, 0xd57, 0xd56, 0xd55, 0xd55, 0xd54, 0xd54, 0xd53, 0xd53, 0xd52, 0xd52, 0xd51, 0xd51, 0xd50, 0xd4f, 0xd4f, 0xd4e, 0xd4e, 0xd4d, 0xd4d, 0xd4c, 0xd4c, 0xd4b, 0xd4a, 0xd4a, 0xd49, 0xd49, 0xd48, 0xd48, 0xd47, 0xd47, 0xd46, 0xd46, 0xd45, 0xd44, 0xd44, 0xd43, 0xd43, 0xd42, 0xd42, 0xd41, 0xd41, 0xd40, 0xd3f, 0xd3f, 0xd3e, 0xd3e, 0xd3d, 0xd3d, 0xd3c, 0xd3c, 0xd3b, 0xd3a, 0xd3a, 0xd39, 0xd39, 0xd38, 0xd38, 0xd37, 0xd36, 0xd36, 0xd35, 0xd35, 0xd34, 0xd34, 0xd33, 0xd33, 0xd32, 0xd31, 0xd31, 0xd30, 0xd30, 0xd2f, 0xd2f, 0xd2e, 0xd2e, 0xd2d, 0xd2c, 0xd2c, 0xd2b, 0xd2b, 0xd2a, 0xd2a, 0xd29, 0xd28, 0xd28, 0xd27, 0xd27, 0xd26, 0xd26, 0xd25, 0xd24, 0xd24, 0xd23, 0xd23, 0xd22, 0xd22, 0xd21, 0xd20, 0xd20, 0xd1f, 0xd1f, 0xd1e, 0xd1e, 0xd1d, 0xd1d, 0xd1c, 0xd1b, 0xd1b, 0xd1a, 0xd1a, 0xd19, 0xd19, 0xd18, 0xd17, 0xd17, 0xd16, 0xd16, 0xd15, 0xd15, 0xd14, 0xd13, 0xd13, 0xd12, 0xd12, 0xd11, 0xd10, 0xd10, 0xd0f, 0xd0f, 0xd0e, 0xd0e, 0xd0d, 0xd0c, 0xd0c, 0xd0b, 0xd0b, 0xd0a, 0xd0a, 0xd09, 0xd08, 0xd08, 0xd07, 0xd07, 0xd06, 0xd06, 0xd05, 0xd04, 0xd04, 0xd03, 0xd03, 0xd02, 0xd01, 0xd01, 0xd00, 0xd00, 0xcff, 0xcff, 0xcfe, 0xcfd, 0xcfd, 0xcfc, 0xcfc, 0xcfb, 0xcfa, 0xcfa, 0xcf9, 0xcf9, 0xcf8, 0xcf8, 0xcf7, 0xcf6, 0xcf6, 0xcf5, 0xcf5, 0xcf4, 0xcf3, 0xcf3, 0xcf2, 0xcf2, 0xcf1, 0xcf1, 0xcf0, 0xcef, 0xcef, 0xcee, 0xcee, 0xced, 0xcec, 0xcec, 0xceb, 0xceb, 0xcea, 0xce9, 0xce9, 0xce8, 0xce8, 0xce7, 0xce7, 0xce6, 0xce5, 0xce5, 0xce4, 0xce4, 0xce3, 0xce2, 0xce2, 0xce1, 0xce1, 0xce0, 0xcdf, 0xcdf, 0xcde, 0xcde, 0xcdd, 0xcdc, 0xcdc, 0xcdb, 0xcdb, 0xcda, 0xcd9, 0xcd9, 0xcd8, 0xcd8, 0xcd7, 0xcd6, 0xcd6, 0xcd5, 0xcd5, 0xcd4, 0xcd3, 0xcd3, 0xcd2, 0xcd2, 0xcd1, 0xcd0, 0xcd0, 0xccf, 0xccf, 0xcce, 0xccd, 0xccd, 0xccc, 0xccc, 0xccb, 0xcca, 0xcca, 0xcc9, 0xcc9, 0xcc8, 0xcc7, 0xcc7, 0xcc6, 0xcc6, 0xcc5, 0xcc4, 0xcc4, 0xcc3, 0xcc3, 0xcc2, 0xcc1, 0xcc1, 0xcc0, 0xcc0, 0xcbf, 0xcbe, 0xcbe, 0xcbd, 0xcbc, 0xcbc, 0xcbb, 0xcbb, 0xcba, 0xcb9, 0xcb9, 0xcb8, 0xcb8, 0xcb7, 0xcb6, 0xcb6, 0xcb5, 0xcb5, 0xcb4, 0xcb3, 0xcb3, 0xcb2, 0xcb1, 0xcb1, 0xcb0, 0xcb0, 0xcaf, 0xcae, 0xcae, 0xcad, 0xcad, 0xcac, 0xcab, 0xcab, 0xcaa, 0xca9, 0xca9, 0xca8, 0xca8, 0xca7, 0xca6, 0xca6, 0xca5, 0xca5, 0xca4, 0xca3, 0xca3, 0xca2, 0xca1, 0xca1, 0xca0, 0xca0, 0xc9f, 0xc9e, 0xc9e, 0xc9d, 0xc9c, 0xc9c, 0xc9b, 0xc9b, 0xc9a, 0xc99, 0xc99, 0xc98, 0xc97, 0xc97, 0xc96, 0xc96, 0xc95, 0xc94, 0xc94, 0xc93, 0xc92, 0xc92, 0xc91, 0xc91, 0xc90, 0xc8f, 0xc8f, 0xc8e, 0xc8d, 0xc8d, 0xc8c, 0xc8c, 0xc8b, 0xc8a, 0xc8a, 0xc89, 0xc88, 0xc88, 0xc87, 0xc86, 0xc86, 0xc85, 0xc85, 0xc84, 0xc83, 0xc83, 0xc82, 0xc81, 0xc81, 0xc80, 0xc80, 0xc7f, 0xc7e, 0xc7e, 0xc7d, 0xc7c, 0xc7c, 0xc7b, 0xc7a, 0xc7a, 0xc79, 0xc79, 0xc78, 0xc77, 0xc77, 0xc76, 0xc75, 0xc75, 0xc74, 0xc73, 0xc73, 0xc72, 0xc71, 0xc71, 0xc70, 0xc70, 0xc6f, 0xc6e, 0xc6e, 0xc6d, 0xc6c, 0xc6c, 0xc6b, 0xc6a, 0xc6a, 0xc69, 0xc68, 0xc68, 0xc67, 0xc67, 0xc66, 0xc65, 0xc65, 0xc64, 0xc63, 0xc63, 0xc62, 0xc61, 0xc61, 0xc60, 0xc5f, 0xc5f, 0xc5e, 0xc5d, 0xc5d, 0xc5c, 0xc5b, 0xc5b, 0xc5a, 0xc5a, 0xc59, 0xc58, 0xc58, 0xc57, 0xc56, 0xc56, 0xc55, 0xc54, 0xc54, 0xc53, 0xc52, 0xc52, 0xc51, 0xc50, 0xc50, 0xc4f, 0xc4e, 0xc4e, 0xc4d, 0xc4c, 0xc4c, 0xc4b, 0xc4a, 0xc4a, 0xc49, 0xc48, 0xc48, 0xc47, 0xc46, 0xc46, 0xc45, 0xc44, 0xc44, 0xc43, 0xc42, 0xc42, 0xc41, 0xc40, 0xc40, 0xc3f, 0xc3e, 0xc3e, 0xc3d, 0xc3d, 0xc3c, 0xc3b, 0xc3b, 0xc3a, 0xc39, 0xc39, 0xc38, 0xc37, 0xc36, 0xc36, 0xc35, 0xc34, 0xc34, 0xc33, 0xc32, 0xc32, 0xc31, 0xc30, 0xc30, 0xc2f, 0xc2e, 0xc2e, 0xc2d, 0xc2c, 0xc2c, 0xc2b, 0xc2a, 0xc2a, 0xc29, 0xc28, 0xc28, 0xc27, 0xc26, 0xc26, 0xc25, 0xc24, 0xc24, 0xc23, 0xc22, 0xc22, 0xc21, 0xc20, 0xc20, 0xc1f, 0xc1e, 0xc1e, 0xc1d, 0xc1c, 0xc1b, 0xc1b, 0xc1a, 0xc19, 0xc19, 0xc18, 0xc17, 0xc17, 0xc16, 0xc15, 0xc15, 0xc14, 0xc13, 0xc13, 0xc12, 0xc11, 0xc11, 0xc10, 0xc0f, 0xc0e, 0xc0e, 0xc0d, 0xc0c, 0xc0c, 0xc0b, 0xc0a, 0xc0a, 0xc09, 0xc08, 0xc08, 0xc07, 0xc06, 0xc06, 0xc05, 0xc04, 0xc03, 0xc03, 0xc02, 0xc01, 0xc01, 0xc00, 0xbff, 0xbff, 0xbfe, 0xbfd, 0xbfc, 0xbfc, 0xbfb, 0xbfa, 0xbfa, 0xbf9, 0xbf8, 0xbf8, 0xbf7, 0xbf6, 0xbf6, 0xbf5, 0xbf4, 0xbf3, 0xbf3, 0xbf2, 0xbf1, 0xbf1, 0xbf0, 0xbef, 0xbef, 0xbee, 0xbed, 0xbec, 0xbec, 0xbeb, 0xbea, 0xbea, 0xbe9, 0xbe8, 0xbe7, 0xbe7, 0xbe6, 0xbe5, 0xbe5, 0xbe4, 0xbe3, 0xbe3, 0xbe2, 0xbe1, 0xbe0, 0xbe0, 0xbdf, 0xbde, 0xbde, 0xbdd, 0xbdc, 0xbdb, 0xbdb, 0xbda, 0xbd9, 0xbd9, 0xbd8, 0xbd7, 0xbd6, 0xbd6, 0xbd5, 0xbd4, 0xbd4, 0xbd3, 0xbd2, 0xbd1, 0xbd1, 0xbd0, 0xbcf, 0xbcf, 0xbce, 0xbcd, 0xbcc, 0xbcc, 0xbcb, 0xbca, 0xbc9, 0xbc9, 0xbc8, 0xbc7, 0xbc7, 0xbc6, 0xbc5, 0xbc4, 0xbc4, 0xbc3, 0xbc2, 0xbc2, 0xbc1, 0xbc0, 0xbbf, 0xbbf, 0xbbe, 0xbbd, 0xbbc, 0xbbc, 0xbbb, 0xbba, 0xbba, 0xbb9, 0xbb8, 0xbb7, 0xbb7, 0xbb6, 0xbb5, 0xbb4, 0xbb4, 0xbb3, 0xbb2, 0xbb1, 0xbb1, 0xbb0, 0xbaf, 0xbaf, 0xbae, 0xbad, 0xbac, 0xbac, 0xbab, 0xbaa, 0xba9, 0xba9, 0xba8, 0xba7, 0xba6, 0xba6, 0xba5, 0xba4, 0xba3, 0xba3, 0xba2, 0xba1, 0xba0, 0xba0, 0xb9f, 0xb9e, 0xb9e, 0xb9d, 0xb9c, 0xb9b, 0xb9b, 0xb9a, 0xb99, 0xb98, 0xb98, 0xb97, 0xb96, 0xb95, 0xb95, 0xb94, 0xb93, 0xb92, 0xb92, 0xb91, 0xb90, 0xb8f, 0xb8f, 0xb8e, 0xb8d, 0xb8c, 0xb8c, 0xb8b, 0xb8a, 0xb89, 0xb89, 0xb88, 0xb87, 0xb86, 0xb86, 0xb85, 0xb84, 0xb83, 0xb82, 0xb82, 0xb81, 0xb80, 0xb7f, 0xb7f, 0xb7e, 0xb7d, 0xb7c, 0xb7c, 0xb7b, 0xb7a, 0xb79, 0xb79, 0xb78, 0xb77, 0xb76, 0xb76, 0xb75, 0xb74, 0xb73, 0xb72, 0xb72, 0xb71, 0xb70, 0xb6f, 0xb6f, 0xb6e, 0xb6d, 0xb6c, 0xb6c, 0xb6b, 0xb6a, 0xb69, 0xb68, 0xb68, 0xb67, 0xb66, 0xb65, 0xb65, 0xb64, 0xb63, 0xb62, 0xb62, 0xb61, 0xb60, 0xb5f, 0xb5e, 0xb5e, 0xb5d, 0xb5c, 0xb5b, 0xb5b, 0xb5a, 0xb59, 0xb58, 0xb57, 0xb57, 0xb56, 0xb55, 0xb54, 0xb53, 0xb53, 0xb52, 0xb51, 0xb50, 0xb50, 0xb4f, 0xb4e, 0xb4d, 0xb4c, 0xb4c, 0xb4b, 0xb4a, 0xb49, 0xb48, 0xb48, 0xb47, 0xb46, 0xb45, 0xb45, 0xb44, 0xb43, 0xb42, 0xb41, 0xb41, 0xb40, 0xb3f, 0xb3e, 0xb3d, 0xb3d, 0xb3c, 0xb3b, 0xb3a, 0xb39, 0xb39, 0xb38, 0xb37, 0xb36, 0xb35, 0xb35, 0xb34, 0xb33, 0xb32, 0xb31, 0xb31, 0xb30, 0xb2f, 0xb2e, 0xb2d, 0xb2d, 0xb2c, 0xb2b, 0xb2a, 0xb29, 0xb29, 0xb28, 0xb27, 0xb26, 0xb25, 0xb25, 0xb24, 0xb23, 0xb22, 0xb21, 0xb20, 0xb20, 0xb1f, 0xb1e, 0xb1d, 0xb1c, 0xb1c, 0xb1b, 0xb1a, 0xb19, 0xb18, 0xb18, 0xb17, 0xb16, 0xb15, 0xb14, 0xb13, 0xb13, 0xb12, 0xb11, 0xb10, 0xb0f, 0xb0f, 0xb0e, 0xb0d, 0xb0c, 0xb0b, 0xb0a, 0xb0a, 0xb09, 0xb08, 0xb07, 0xb06, 0xb05, 0xb05, 0xb04, 0xb03, 0xb02, 0xb01, 0xb00, 0xb00, 0xaff, 0xafe, 0xafd, 0xafc, 0xafb, 0xafb, 0xafa, 0xaf9, 0xaf8, 0xaf7, 0xaf6, 0xaf6, 0xaf5, 0xaf4, 0xaf3, 0xaf2, 0xaf1, 0xaf1, 0xaf0, 0xaef, 0xaee, 0xaed, 0xaec, 0xaec, 0xaeb, 0xaea, 0xae9, 0xae8, 0xae7, 0xae7, 0xae6, 0xae5, 0xae4, 0xae3, 0xae2, 0xae1, 0xae1, 0xae0, 0xadf, 0xade, 0xadd, 0xadc, 0xadb, 0xadb, 0xada, 0xad9, 0xad8, 0xad7, 0xad6, 0xad6, 0xad5, 0xad4, 0xad3, 0xad2, 0xad1, 0xad0, 0xad0, 0xacf, 0xace, 0xacd, 0xacc, 0xacb, 0xaca, 0xac9, 0xac9, 0xac8, 0xac7, 0xac6, 0xac5, 0xac4, 0xac3, 0xac3, 0xac2, 0xac1, 0xac0, 0xabf, 0xabe, 0xabd, 0xabd, 0xabc, 0xabb, 0xaba, 0xab9, 0xab8, 0xab7, 0xab6, 0xab6, 0xab5, 0xab4, 0xab3, 0xab2, 0xab1, 0xab0, 0xaaf, 0xaaf, 0xaae, 0xaad, 0xaac, 0xaab, 0xaaa, 0xaa9, 0xaa8, 0xaa7, 0xaa7, 0xaa6, 0xaa5, 0xaa4, 0xaa3, 0xaa2, 0xaa1, 0xaa0, 0xaa0, 0xa9f, 0xa9e, 0xa9d, 0xa9c, 0xa9b, 0xa9a, 0xa99, 0xa98, 0xa97, 0xa97, 0xa96, 0xa95, 0xa94, 0xa93, 0xa92, 0xa91, 0xa90, 0xa8f, 0xa8f, 0xa8e, 0xa8d, 0xa8c, 0xa8b, 0xa8a, 0xa89, 0xa88, 0xa87, 0xa86, 0xa86, 0xa85, 0xa84, 0xa83, 0xa82, 0xa81, 0xa80, 0xa7f, 0xa7e, 0xa7d, 0xa7c, 0xa7c, 0xa7b, 0xa7a, 0xa79, 0xa78, 0xa77, 0xa76, 0xa75, 0xa74, 0xa73, 0xa72, 0xa72, 0xa71, 0xa70, 0xa6f, 0xa6e, 0xa6d, 0xa6c, 0xa6b, 0xa6a, 0xa69, 0xa68, 0xa67, 0xa66, 0xa66, 0xa65, 0xa64, 0xa63, 0xa62, 0xa61, 0xa60, 0xa5f, 0xa5e, 0xa5d, 0xa5c, 0xa5b, 0xa5a, 0xa59, 0xa59, 0xa58, 0xa57, 0xa56, 0xa55, 0xa54, 0xa53, 0xa52, 0xa51, 0xa50, 0xa4f, 0xa4e, 0xa4d, 0xa4c, 0xa4b, 0xa4a, 0xa4a, 0xa49, 0xa48, 0xa47, 0xa46, 0xa45, 0xa44, 0xa43, 0xa42, 0xa41, 0xa40, 0xa3f, 0xa3e, 0xa3d, 0xa3c, 0xa3b, 0xa3a, 0xa39, 0xa38, 0xa38, 0xa37, 0xa36, 0xa35, 0xa34, 0xa33, 0xa32, 0xa31, 0xa30, 0xa2f, 0xa2e, 0xa2d, 0xa2c, 0xa2b, 0xa2a, 0xa29, 0xa28, 0xa27, 0xa26, 0xa25, 0xa24, 0xa23, 0xa22, 0xa21, 0xa20, 0xa1f, 0xa1e, 0xa1e, 0xa1d, 0xa1c, 0xa1b, 0xa1a, 0xa19, 0xa18, 0xa17, 0xa16, 0xa15, 0xa14, 0xa13, 0xa12, 0xa11, 0xa10, 0xa0f, 0xa0e, 0xa0d, 0xa0c, 0xa0b, 0xa0a, 0xa09, 0xa08, 0xa07, 0xa06, 0xa05, 0xa04, 0xa03, 0xa02, 0xa01, 0xa00, 0x9ff, 0x9fe, 0x9fd, 0x9fc, 0x9fb, 0x9fa, 0x9f9, 0x9f8, 0x9f7, 0x9f6, 0x9f5, 0x9f4, 0x9f3, 0x9f2, 0x9f1, 0x9f0, 0x9ef, 0x9ee, 0x9ed, 0x9ec, 0x9eb, 0x9ea, 0x9e9, 0x9e8, 0x9e7, 0x9e6, 0x9e5, 0x9e4, 0x9e3, 0x9e2, 0x9e1, 0x9e0, 0x9df, 0x9de, 0x9dd, 0x9dc, 0x9db, 0x9da, 0x9d9, 0x9d8, 0x9d7, 0x9d6, 0x9d5, 0x9d4, 0x9d3, 0x9d2, 0x9d1, 0x9d0, 0x9cf, 0x9ce, 0x9cd, 0x9cc, 0x9cb, 0x9c9, 0x9c8, 0x9c7, 0x9c6, 0x9c5, 0x9c4, 0x9c3, 0x9c2, 0x9c1, 0x9c0, 0x9bf, 0x9be, 0x9bd, 0x9bc, 0x9bb, 0x9ba, 0x9b9, 0x9b8, 0x9b7, 0x9b6, 0x9b5, 0x9b4, 0x9b3, 0x9b2, 0x9b1, 0x9af, 0x9ae, 0x9ad, 0x9ac, 0x9ab, 0x9aa, 0x9a9, 0x9a8, 0x9a7, 0x9a6, 0x9a5, 0x9a4, 0x9a3, 0x9a2, 0x9a1, 0x9a0, 0x99f, 0x99e, 0x99c, 0x99b, 0x99a, 0x999, 0x998, 0x997, 0x996, 0x995, 0x994, 0x993, 0x992, 0x991, 0x990, 0x98f, 0x98e, 0x98c, 0x98b, 0x98a, 0x989, 0x988, 0x987, 0x986, 0x985, 0x984, 0x983, 0x982, 0x981, 0x980, 0x97e, 0x97d, 0x97c, 0x97b, 0x97a, 0x979, 0x978, 0x977, 0x976, 0x975, 0x974, 0x972, 0x971, 0x970, 0x96f, 0x96e, 0x96d, 0x96c, 0x96b, 0x96a, 0x969, 0x967, 0x966, 0x965, 0x964, 0x963, 0x962, 0x961, 0x960, 0x95f, 0x95d, 0x95c, 0x95b, 0x95a, 0x959, 0x958, 0x957, 0x956, 0x955, 0x953, 0x952, 0x951, 0x950, 0x94f, 0x94e, 0x94d, 0x94c, 0x94a, 0x949, 0x948, 0x947, 0x946, 0x945, 0x944, 0x943, 0x941, 0x940, 0x93f, 0x93e, 0x93d, 0x93c, 0x93b, 0x939, 0x938, 0x937, 0x936, 0x935, 0x934, 0x933, 0x931, 0x930, 0x92f, 0x92e, 0x92d, 0x92c, 0x92b, 0x929, 0x928, 0x927, 0x926, 0x925, 0x924, 0x923, 0x921, 0x920, 0x91f, 0x91e, 0x91d, 0x91c, 0x91a, 0x919, 0x918, 0x917, 0x916, 0x915, 0x913, 0x912, 0x911, 0x910, 0x90f, 0x90e, 0x90c, 0x90b, 0x90a, 0x909, 0x908, 0x906, 0x905, 0x904, 0x903, 0x902, 0x901, 0x8ff, 0x8fe, 0x8fd, 0x8fc, 0x8fb, 0x8f9, 0x8f8, 0x8f7, 0x8f6, 0x8f5, 0x8f3, 0x8f2, 0x8f1, 0x8f0, 0x8ef, 0x8ed, 0x8ec, 0x8eb, 0x8ea, 0x8e9, 0x8e7, 0x8e6, 0x8e5, 0x8e4, 0x8e3, 0x8e1, 0x8e0, 0x8df, 0x8de, 0x8dd, 0x8db, 0x8da, 0x8d9, 0x8d8, 0x8d6, 0x8d5, 0x8d4, 0x8d3, 0x8d2, 0x8d0, 0x8cf, 0x8ce, 0x8cd, 0x8cb, 0x8ca, 0x8c9, 0x8c8, 0x8c7, 0x8c5, 0x8c4, 0x8c3, 0x8c2, 0x8c0, 0x8bf, 0x8be, 0x8bd, 0x8bb, 0x8ba, 0x8b9, 0x8b8, 0x8b6, 0x8b5, 0x8b4, 0x8b3, 0x8b1, 0x8b0, 0x8af, 0x8ae, 0x8ac, 0x8ab, 0x8aa, 0x8a9, 0x8a7, 0x8a6, 0x8a5, 0x8a4, 0x8a2, 0x8a1, 0x8a0, 0x89f, 0x89d, 0x89c, 0x89b, 0x899, 0x898, 0x897, 0x896, 0x894, 0x893, 0x892, 0x890, 0x88f, 0x88e, 0x88d, 0x88b, 0x88a, 0x889, 0x888, 0x886, 0x885, 0x884, 0x882, 0x881, 0x880, 0x87e, 0x87d, 0x87c, 0x87b, 0x879, 0x878, 0x877, 0x875, 0x874, 0x873, 0x871, 0x870, 0x86f, 0x86d, 0x86c, 0x86b, 0x86a, 0x868, 0x867, 0x866, 0x864, 0x863, 0x862, 0x860, 0x85f, 0x85e, 0x85c, 0x85b, 0x85a, 0x858, 0x857, 0x856, 0x854, 0x853, 0x852, 0x850, 0x84f, 0x84e, 0x84c, 0x84b, 0x84a, 0x848, 0x847, 0x846, 0x844, 0x843, 0x842, 0x840, 0x83f, 0x83d, 0x83c, 0x83b, 0x839, 0x838, 0x837, 0x835, 0x834, 0x833, 0x831, 0x830, 0x82e, 0x82d, 0x82c, 0x82a, 0x829, 0x828, 0x826, 0x825, 0x823, 0x822, 0x821, 0x81f, 0x81e, 0x81c, 0x81b, 0x81a, 0x818, 0x817, 0x816, 0x814, 0x813, 0x811, 0x810, 0x80f, 0x80d, 0x80c, 0x80a, 0x809, 0x808, 0x806, 0x805, 0x803, 0x802, 0x800, 0x7ff, 0x7fe, 0x7fc, 0x7fb, 0x7f9, 0x7f8, 0x7f7, 0x7f5, 0x7f4, 0x7f2, 0x7f1, 0x7ef, 0x7ee, 0x7ec, 0x7eb, 0x7ea, 0x7e8, 0x7e7, 0x7e5, 0x7e4, 0x7e2, 0x7e1, 0x7e0, 0x7de, 0x7dd, 0x7db, 0x7da, 0x7d8, 0x7d7, 0x7d5, 0x7d4, 0x7d2, 0x7d1, 0x7cf, 0x7ce, 0x7cd, 0x7cb, 0x7ca, 0x7c8, 0x7c7, 0x7c5, 0x7c4, 0x7c2, 0x7c1, 0x7bf, 0x7be, 0x7bc, 0x7bb, 0x7b9, 0x7b8, 0x7b6, 0x7b5, 0x7b3, 0x7b2, 0x7b0, 0x7af, 0x7ad, 0x7ac, 0x7aa, 0x7a9, 0x7a7, 0x7a6, 0x7a4, 0x7a3, 0x7a1, 0x7a0, 0x79e, 0x79d, 0x79b, 0x79a, 0x798, 0x797, 0x795, 0x794, 0x792, 0x791, 0x78f, 0x78e, 0x78c, 0x78a, 0x789, 0x787, 0x786, 0x784, 0x783, 0x781, 0x780, 0x77e, 0x77d, 0x77b, 0x779, 0x778, 0x776, 0x775, 0x773, 0x772, 0x770, 0x76f, 0x76d, 0x76b, 0x76a, 0x768, 0x767, 0x765, 0x764, 0x762, 0x760, 0x75f, 0x75d, 0x75c, 0x75a, 0x758, 0x757, 0x755, 0x754, 0x752, 0x750, 0x74f, 0x74d, 0x74c, 0x74a, 0x748, 0x747, 0x745, 0x744, 0x742, 0x740, 0x73f, 0x73d, 0x73b, 0x73a, 0x738, 0x737, 0x735, 0x733, 0x732, 0x730, 0x72e, 0x72d, 0x72b, 0x729, 0x728, 0x726, 0x725, 0x723, 0x721, 0x720, 0x71e, 0x71c, 0x71b, 0x719, 0x717, 0x716, 0x714, 0x712, 0x711, 0x70f, 0x70d, 0x70c, 0x70a, 0x708, 0x707, 0x705, 0x703, 0x701, 0x700, 0x6fe, 0x6fc, 0x6fb, 0x6f9, 0x6f7, 0x6f6, 0x6f4, 0x6f2, 0x6f0, 0x6ef, 0x6ed, 0x6eb, 0x6ea, 0x6e8, 0x6e6, 0x6e4, 0x6e3, 0x6e1, 0x6df, 0x6de, 0x6dc, 0x6da, 0x6d8, 0x6d7, 0x6d5, 0x6d3, 0x6d1, 0x6d0, 0x6ce, 0x6cc, 0x6ca, 0x6c9, 0x6c7, 0x6c5, 0x6c3, 0x6c2, 0x6c0, 0x6be, 0x6bc, 0x6ba, 0x6b9, 0x6b7, 0x6b5, 0x6b3, 0x6b2, 0x6b0, 0x6ae, 0x6ac, 0x6aa, 0x6a9, 0x6a7, 0x6a5, 0x6a3, 0x6a1, 0x6a0, 0x69e, 0x69c, 0x69a, 0x698, 0x696, 0x695, 0x693, 0x691, 0x68f, 0x68d, 0x68b, 0x68a, 0x688, 0x686, 0x684, 0x682, 0x680, 0x67f, 0x67d, 0x67b, 0x679, 0x677, 0x675, 0x673, 0x672, 0x670, 0x66e, 0x66c, 0x66a, 0x668, 0x666, 0x664, 0x663, 0x661, 0x65f, 0x65d, 0x65b, 0x659, 0x657, 0x655, 0x653, 0x651, 0x650, 0x64e, 0x64c, 0x64a, 0x648, 0x646, 0x644, 0x642, 0x640, 0x63e, 0x63c, 0x63a, 0x638, 0x636, 0x634, 0x633, 0x631, 0x62f, 0x62d, 0x62b, 0x629, 0x627, 0x625, 0x623, 0x621, 0x61f, 0x61d, 0x61b, 0x619, 0x617, 0x615, 0x613, 0x611, 0x60f, 0x60d, 0x60b, 0x609, 0x607, 0x605, 0x603, 0x601, 0x5ff, 0x5fd, 0x5fb, 0x5f9, 0x5f7, 0x5f5, 0x5f3, 0x5f1, 0x5ef, 0x5ec, 0x5ea, 0x5e8, 0x5e6, 0x5e4, 0x5e2, 0x5e0, 0x5de, 0x5dc, 0x5da, 0x5d8, 0x5d6, 0x5d4, 0x5d2, 0x5cf, 0x5cd, 0x5cb, 0x5c9, 0x5c7, 0x5c5, 0x5c3, 0x5c1, 0x5bf, 0x5bd, 0x5ba, 0x5b8, 0x5b6, 0x5b4, 0x5b2, 0x5b0, 0x5ae, 0x5ab, 0x5a9, 0x5a7, 0x5a5, 0x5a3, 0x5a1, 0x59f, 0x59c, 0x59a, 0x598, 0x596, 0x594, 0x591, 0x58f, 0x58d, 0x58b, 0x589, 0x587, 0x584, 0x582, 0x580, 0x57e, 0x57b, 0x579, 0x577, 0x575, 0x573, 0x570, 0x56e, 0x56c, 0x56a, 0x567, 0x565, 0x563, 0x561, 0x55e, 0x55c, 0x55a, 0x558, 0x555, 0x553, 0x551, 0x54e, 0x54c, 0x54a, 0x548, 0x545, 0x543, 0x541, 0x53e, 0x53c, 0x53a, 0x537, 0x535, 0x533, 0x530, 0x52e, 0x52c, 0x529, 0x527, 0x525, 0x522, 0x520, 0x51e, 0x51b, 0x519, 0x516, 0x514, 0x512, 0x50f, 0x50d, 0x50a, 0x508, 0x506, 0x503, 0x501, 0x4fe, 0x4fc, 0x4fa, 0x4f7, 0x4f5, 0x4f2, 0x4f0, 0x4ed, 0x4eb, 0x4e8, 0x4e6, 0x4e4, 0x4e1, 0x4df, 0x4dc, 0x4da, 0x4d7, 0x4d5, 0x4d2, 0x4d0, 0x4cd, 0x4cb, 0x4c8, 0x4c6, 0x4c3, 0x4c1, 0x4be, 0x4bc, 0x4b9, 0x4b6, 0x4b4, 0x4b1, 0x4af, 0x4ac, 0x4aa, 0x4a7, 0x4a5, 0x4a2, 0x49f, 0x49d, 0x49a, 0x498, 0x495, 0x492, 0x490, 0x48d, 0x48b, 0x488, 0x485, 0x483, 0x480, 0x47d, 0x47b, 0x478, 0x475, 0x473, 0x470, 0x46d, 0x46b, 0x468, 0x465, 0x463, 0x460, 0x45d, 0x45b, 0x458, 0x455, 0x452, 0x450, 0x44d, 0x44a, 0x447, 0x445, 0x442, 0x43f, 0x43c, 0x43a, 0x437, 0x434, 0x431, 0x42f, 0x42c, 0x429, 0x426, 0x423, 0x420, 0x41e, 0x41b, 0x418, 0x415, 0x412, 0x40f, 0x40d, 0x40a, 0x407, 0x404, 0x401, 0x3fe, 0x3fb, 0x3f8, 0x3f5, 0x3f3, 0x3f0, 0x3ed, 0x3ea, 0x3e7, 0x3e4, 0x3e1, 0x3de, 0x3db, 0x3d8, 0x3d5, 0x3d2, 0x3cf, 0x3cc, 0x3c9, 0x3c6, 0x3c3, 0x3c0, 0x3bd, 0x3ba, 0x3b7, 0x3b4, 0x3b1, 0x3ae, 0x3ab, 0x3a8, 0x3a5, 0x3a2, 0x39e, 0x39b, 0x398, 0x395, 0x392, 0x38f, 0x38c, 0x389, 0x386, 0x382, 0x37f, 0x37c, 0x379, 0x376, 0x373, 0x36f, 0x36c, 0x369, 0x366, 0x363, 0x35f, 0x35c, 0x359, 0x356, 0x352, 0x34f, 0x34c, 0x349, 0x345, 0x342, 0x33f, 0x33b, 0x338, 0x335, 0x331, 0x32e, 0x32b, 0x327, 0x324, 0x321, 0x31d, 0x31a, 0x316, 0x313, 0x310, 0x30c, 0x309, 0x305, 0x302, 0x2fe, 0x2fb, 0x2f8, 0x2f4, 0x2f1, 0x2ed, 0x2ea, 0x2e6, 0x2e3, 0x2df, 0x2db, 0x2d8, 0x2d4, 0x2d1, 0x2cd, 0x2ca, 0x2c6, 0x2c2, 0x2bf, 0x2bb, 0x2b8, 0x2b4, 0x2b0, 0x2ad, 0x2a9, 0x2a5, 0x2a2, 0x29e, 0x29a, 0x297, 0x293, 0x28f, 0x28b, 0x288, 0x284, 0x280, 0x27c, 0x279, 0x275, 0x271, 0x26d, 0x269, 0x265, 0x262, 0x25e, 0x25a, 0x256, 0x252, 0x24e, 0x24a, 0x246, 0x242, 0x23e, 0x23b, 0x237, 0x233, 0x22f, 0x22b, 0x227, 0x223, 0x21f, 0x21b, 0x216, 0x212, 0x20e, 0x20a, 0x206, 0x202, 0x1fe, 0x1fa, 0x1f6, 0x1f1, 0x1ed, 0x1e9, 0x1e5, 0x1e1, 0x1dc, 0x1d8, 0x1d4, 0x1d0, 0x1cb, 0x1c7, 0x1c3, 0x1bf, 0x1ba, 0x1b6, 0x1b2, 0x1ad, 0x1a9, 0x1a4, 0x1a0, 0x19c, 0x197, 0x193, 0x18e, 0x18a, 0x185, 0x181, 0x17c, 0x178, 0x173, 0x16f, 0x16a, 0x166, 0x161, 0x15c, 0x158, 0x153, 0x14e, 0x14a, 0x145, 0x140, 0x13c, 0x137, 0x132, 0x12d, 0x129, 0x124, 0x11f, 0x11a, 0x115, 0x110, 0x10b, 0x107, 0x102, 0xfd, 0xf8, 0xf3, 0xee, 0xe9, 0xe4, 0xdf, 0xda, 0xd5, 0xd0, 0xca, 0xc5, 0xc0, 0xbb, 0xb6, 0xb1, 0xab, 0xa6, 0xa1, 0x9c, 0x96, 0x91, 0x8c, 0x86, 0x81, 0x7c, 0x76, 0x71, 0x6b, 0x66, 0x60, 0x5b, 0x55, 0x50, 0x4a, 0x45, 0x3f, 0x39, 0x34, 0x2e, 0x28, 0x22, 0x1d, 0x17, 0x11, 0xb, 0x5, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
};
#endif

RAMN_SENSORS_ChassisSensors_t RAMN_SENSORS_CHASSIS = {
		.steeringPotentiometer = 0x7FF,
		.sidebrakeSwitch = RAMN_SIDEBRAKE_UNKNOWN,
		.lightsSwitch = RAMN_LIGHTSWITCH_FLOATING
};

#endif

#if defined(EXPANSION_POWERTRAIN)

RAMN_SENSORS_PowertrainSensors_t RAMN_SENSORS_POWERTRAIN_PREVIOUS = {
		.brakePotentiometer = 0U,
		.accelPotentiometer = 0U,
		.shiftJoystick = RAMN_SHIFT_RELEASED,
		.gear = 0U
};

RAMN_SENSORS_PowertrainSensors_t RAMN_SENSORS_POWERTRAIN = {
		.brakePotentiometer = 0U,
		.accelPotentiometer = 0U,
		.shiftJoystick = RAMN_SHIFT_RELEASED,
		.gear = 0U
};

#endif

#if defined(EXPANSION_BODY)

RAMN_SENSORS_BodySensors_t RAMN_SENSORS_BODY_PREVIOUS;
RAMN_SENSORS_BodySensors_t RAMN_SENSORS_BODY = {
		.engineKey = RAMN_ENGINEKEY_FLOATING
};
#endif

// Private Functions  --------------------------------------------

#if defined(EXPANSION_POWERTRAIN)

static RAMN_LeverState_t SENSORS_ConvertLeverADC(uint16_t adcval)
{
	RAMN_LeverState_t         result = RAMN_SHIFT_UNKNOWNSTATE;
	if      (adcval >= 0x0D00)  result = RAMN_SHIFT_RELEASED;
	else if (adcval >= 0x0C00)  result = RAMN_SHIFT_DOWN;
	else if (adcval >= 0x0B00)  result = RAMN_SHIFT_RIGHT;
	else if (adcval >= 0x0A00)  result = RAMN_SHIFT_LEFT;
	else if (adcval >= 0x0700)  result = RAMN_SHIFT_PUSH;
	else if (adcval <= 0x0100)  result = RAMN_SHIFT_UP;
	return result;
}

static void SENSORS_UpdatePowertrain(uint32_t tick)
{
	RAMN_SENSORS_POWERTRAIN.brakePotentiometer = RAMN_SENSORS_ADCValues[0];
	RAMN_SENSORS_POWERTRAIN.accelPotentiometer = RAMN_SENSORS_ADCValues[1];
	RAMN_SENSORS_POWERTRAIN.shiftJoystick = SENSORS_ConvertLeverADC(RAMN_SENSORS_ADCValues[2]);

	if (tick - lastUpdateTick > 100U ) //TODO: replace this by better debounce
	{
		lastUpdateTick = tick;
		if((RAMN_SENSORS_POWERTRAIN.shiftJoystick != RAMN_SHIFT_RELEASED) && (RAMN_SENSORS_POWERTRAIN_PREVIOUS.shiftJoystick == RAMN_SHIFT_RELEASED))
		{
			if (RAMN_SENSORS_POWERTRAIN.shiftJoystick == RAMN_SHIFT_UP)
			{
				RAMN_SENSORS_POWERTRAIN.gear = ((RAMN_SENSORS_POWERTRAIN.gear < MAX_GEAR_VALUE) || (RAMN_SENSORS_POWERTRAIN.gear == 0xFF)) ? (((uint8_t)RAMN_SENSORS_POWERTRAIN.gear)+1)&0xFF : RAMN_SENSORS_POWERTRAIN.gear;
			}
			if (RAMN_SENSORS_POWERTRAIN.shiftJoystick == RAMN_SHIFT_DOWN)
			{
				RAMN_SENSORS_POWERTRAIN.gear = (RAMN_SENSORS_POWERTRAIN.gear <= MAX_GEAR_VALUE) ? (((uint8_t)RAMN_SENSORS_POWERTRAIN.gear)-1)&0xFF : RAMN_SENSORS_POWERTRAIN.gear;
			}
			if (RAMN_SENSORS_POWERTRAIN.shiftJoystick == RAMN_SHIFT_RIGHT)  RAMN_SENSORS_POWERTRAIN.turnIndicatorRequest = (RAMN_SENSORS_POWERTRAIN.turnIndicatorRequest ^ 0x1)&0xFF;
			if (RAMN_SENSORS_POWERTRAIN.shiftJoystick == RAMN_SHIFT_LEFT)   RAMN_SENSORS_POWERTRAIN.turnIndicatorRequest = (RAMN_SENSORS_POWERTRAIN.turnIndicatorRequest ^ 0x100)&0xFF00;
		}
		RAMN_SENSORS_POWERTRAIN_PREVIOUS = RAMN_SENSORS_POWERTRAIN;
	}

	if (RAMN_SENSORS_POWERTRAIN.shiftJoystick == RAMN_SHIFT_PUSH) RAMN_SENSORS_POWERTRAIN.hornRequest = True;
	else RAMN_SENSORS_POWERTRAIN.hornRequest = False;
}

#endif

#if defined(EXPANSION_CHASSIS)

static RAMN_LightSwitchState_t SENSORS_ConvertLightSwitchADC(uint16_t adcval)
{
	RAMN_LeverState_t         result = RAMN_LIGHTSWITCH_UNKNOWNSTATE;
	if      (adcval >= 0x0B00)  result = RAMN_LIGHTSWITCH_POS1;
	else if (adcval >= 0x0700)  result = RAMN_LIGHTSWITCH_POS2;
	else if (adcval >= 0x0300)  result = RAMN_LIGHTSWITCH_POS3;
	else if (adcval <= 0x0100)  result = RAMN_LIGHTSWITCH_POS4;
	return result;
}

static RAMN_SideBrakeState_t SENSORS_ConvertSideBrakeADC(uint16_t adcval)
{
	if (adcval < 0x700) return RAMN_SIDEBRAKE_DOWN;
	else return RAMN_SIDEBRAKE_UP;
}

static uint16_t SENSORS_LogToLinear(uint16_t val ) {
	return loglookupt[val];
}

static void SENSORS_UpdateChassis(uint32_t tick)
{
	RAMN_LightSwitchState_t currentLightSwitch;
#ifdef CHASSIS_LOGARITHMIC_POTENTIOMETER
	RAMN_SENSORS_CHASSIS.steeringPotentiometer = SENSORS_LogToLinear(RAMN_SENSORS_ADCValues[0]);
#else
	RAMN_SENSORS_CHASSIS.steeringPotentiometer = (0xFFF-RAMN_SENSORS_ADCValues[0]);
#endif
	RAMN_SENSORS_CHASSIS.sidebrakeSwitch = SENSORS_ConvertSideBrakeADC(RAMN_SENSORS_ADCValues[1]);

	currentLightSwitch = SENSORS_ConvertLightSwitchADC(RAMN_SENSORS_ADCValues[2]);

	if (currentLightSwitch != RAMN_SENSORS_CHASSIS.lightsSwitch)
	{
		if (tick - lastUpdateTick > 250U) //TODO: replace this by better switch debounce
		{
			lastUpdateTick = tick;
			RAMN_SENSORS_CHASSIS.lightsSwitch = currentLightSwitch;
		}
	}
	else
	{
		lastUpdateTick = tick;
		RAMN_SENSORS_CHASSIS.lightsSwitch = currentLightSwitch;
	}
}

#endif


#if defined(EXPANSION_BODY)

static RAMN_EngineKeyState_t SENSORS_ConvertEngineKeyADC(uint16_t adcval)
{
	if (adcval <= 0x300) return RAMN_ENGINEKEY_LEFT;
	else if (adcval <= 0xA00) return RAMN_ENGINEKEY_MIDDLE;
	else return RAMN_ENGINEKEY_RIGHT;
}

static void updateBody(uint32_t tick)
{
	RAMN_SENSORS_BODY.engineKey =SENSORS_ConvertEngineKeyADC(RAMN_SENSORS_ADCValues[1]);
}

#endif


//Exported --------------------------------------------

void RAMN_SENSORS_Init(void)
{
	lastUpdateTick = 0U;
}

void RAMN_SENSORS_Update(uint32_t tick)
{

#if defined(EXPANSION_POWERTRAIN)
	SENSORS_UpdatePowertrain(tick);
#endif

#if defined(EXPANSION_CHASSIS)
	SENSORS_UpdateChassis(tick);
#endif

#if defined(EXPANSION_BODY)
	RAMN_SENSORS_BODY_PREVIOUS = RAMN_SENSORS_BODY;
	updateBody(tick);
#endif

}



