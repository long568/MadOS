#include "CfgUser.h"
#include "power.h"

#define BUF_SIZE 10

static uint16_t adc_rc[BUF_SIZE];

static void adc_init(void);
static uint16_t adc_sampl(void);

MadBool pwr_init(void)
{
    adc_init();
    return MTRUE;
}

static void adc_init(void)
{
    LL_DMA_InitTypeDef     dma;
    LL_GPIO_InitTypeDef    GPIO_InitStruct;
    LL_ADC_InitTypeDef     ADC_InitStruct;
    LL_ADC_REG_InitTypeDef ADC_REG_InitStruct;

    /* Peripheral clock enable */
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_ADC);

    /* ADCn GPIO Configuration */
    GPIO_InitStruct.Pin = GPIN_ADC;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(GPIO_ADC, &GPIO_InitStruct);

    /* ADCn DMA Init */
    dma.PeriphOrM2MSrcAddress  = LL_ADC_DMA_GetRegAddr(ADCn,LL_ADC_DMA_REG_REGULAR_DATA);
    dma.MemoryOrM2MDstAddress  = (MadU32)adc_rc;
    dma.Direction              = LL_DMA_DIRECTION_PERIPH_TO_MEMORY;
    dma.Mode                   = LL_DMA_MODE_CIRCULAR;
    dma.PeriphOrM2MSrcIncMode  = LL_DMA_PERIPH_NOINCREMENT;
    dma.MemoryOrM2MDstIncMode  = LL_DMA_MEMORY_INCREMENT;
    dma.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_HALFWORD;
    dma.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_HALFWORD;
    dma.NbData                 = BUF_SIZE;
    dma.PeriphRequest          = LL_DMAMUX_REQ_ADC1;
    dma.Priority               = LL_DMA_PRIORITY_LOW;
    LL_DMA_Init(ADC_DMA, ADC_DMA_CHL, &dma);
    LL_DMA_EnableChannel(ADC_DMA, ADC_DMA_CHL);

    /* Enable internal regulator it manually */
    LL_ADC_EnableInternalRegulator(ADCn);
    
    /* Configure Regular Channel */
    LL_ADC_REG_SetSequencerChAdd(ADCn, ADC_CHL);

    /* Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion) */
    ADC_InitStruct.Clock         = LL_ADC_CLOCK_SYNC_PCLK_DIV4;
    ADC_InitStruct.Resolution    = LL_ADC_RESOLUTION_12B;
    ADC_InitStruct.DataAlignment = LL_ADC_DATA_ALIGN_RIGHT;
    ADC_InitStruct.LowPowerMode  = LL_ADC_LP_MODE_NONE;
    LL_ADC_Init(ADCn, &ADC_InitStruct);

    ADC_REG_InitStruct.TriggerSource    = LL_ADC_REG_TRIG_SOFTWARE;
    ADC_REG_InitStruct.SequencerDiscont = LL_ADC_REG_SEQ_DISCONT_DISABLE;
    ADC_REG_InitStruct.ContinuousMode   = LL_ADC_REG_CONV_CONTINUOUS;
    ADC_REG_InitStruct.DMATransfer      = LL_ADC_REG_DMA_TRANSFER_UNLIMITED;
    ADC_REG_InitStruct.Overrun          = LL_ADC_REG_OVR_DATA_OVERWRITTEN;
    LL_ADC_REG_Init(ADCn, &ADC_REG_InitStruct);

    LL_ADC_SetOverSamplingScope(ADCn, LL_ADC_OVS_DISABLE);
    LL_ADC_SetTriggerFrequencyMode(ADCn, LL_ADC_CLOCK_FREQ_MODE_HIGH);
    LL_ADC_SetSamplingTimeCommonChannels(ADCn, LL_ADC_SAMPLINGTIME_COMMON_1, LL_ADC_SAMPLINGTIME_160CYCLES_5);
    LL_ADC_DisableIT_EOC(ADCn);
    LL_ADC_DisableIT_EOS(ADCn);
    LL_ADC_REG_SetSequencerConfigurable(ADCn, LL_ADC_REG_SEQ_FIXED);
    LL_ADC_REG_SetSequencerScanDirection(ADCn, LL_ADC_REG_SEQ_SCAN_DIR_FORWARD);

    LL_ADC_StartCalibration(ADCn);
    while(LL_ADC_IsCalibrationOnGoing(ADCn));
    LL_ADC_Enable(ADCn);
}

static uint16_t adc_sampl(void)
{
    uint8_t  imax, imin;
    uint16_t max, min;
    uint16_t tmp[4];
    uint16_t rc;

    rc = 0;

    for(uint8_t i = 0; i < 4; i++) {
        LL_ADC_REG_StartConversion(ADCn);
        madTimeDly(5);
        LL_ADC_REG_StopConversion(ADCn);

        max  = adc_rc[0];
        min  = adc_rc[0];
        imax = 0;
        imin = 0;

        for(uint8_t j = 1; j < 10; j++) {
            if(max < adc_rc[j]) {
                max = adc_rc[j];
                imax = j;
            }
            if(min > adc_rc[j]) {
                min = adc_rc[j];
                imin = j;
            }
        }

        if(imax == imin) {
            tmp[i] = max;
        } else {
            tmp[i] = 0;
            for(uint8_t j = 0; j < 10; j++) {
                if(imax == j || imin == j) {
                    continue;
                }
                tmp[i] += adc_rc[j];
            }
            tmp[i] >>= 3;
        }

        rc += tmp[i];
    }

    rc >>= 2;
    return rc;
}

uint8_t pwr_quantity(void)
{
    uint8_t rc;
    uint32_t sampl = (uint32_t)adc_sampl();

    if(sampl > PWR_N_FULL - 1) {
        rc = 100;
    } else if(sampl < PWR_N_ZERO + 1) {
        rc = 0;
    } else {
        rc = (sampl - PWR_N_ZERO) * 100 / (PWR_N_FULL - PWR_N_ZERO);
    }

    return rc;
}
