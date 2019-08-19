///* Standard includes. */
#include <stdio.h>
#include <string.h>
//
///* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
//
///* MQTT include. */
#include "aws_mqtt_agent.h"
//
///* Demo configurations. */
#include "aws_demo_config.h"
//
///* Required to get the broker address and port. */
#include "aws_clientcredential.h"
//
///* Required for shadow APIs. */
#include "aws_shadow.h"

/* Demo configurations. */
#include "aws_demo_config.h"
#include "renesas_provisioning.h"

/* Required for shadow demo. */
#include "renesas_demo.h"
#include "jsmn.h"
#include "renesas_sensors.h"

/* Task names. */
#define renesasDemoCHAR_TASK_NAME           "Re-IOT-%d"
#define renesasDemoUPDATE_TASK_NAME         "ReDemoUpdt"

/* Format string and length of the initial reported state, "red". */
#define renesasDemoINITIAL_REPORT_FORMAT    "{\"%s\": \"red\"}"
#define renesasDemoINITIAL_REPORT_BUFFER_LENGTH \
    ( sizeof( renesasDemoCHAR_TASK_NAME ) + sizeof( renesasDemoINITIAL_REPORT_FORMAT ) )

/* JSON formats used in the Shadow tasks. Note the inclusion of the "clientToken"
 * key, which is REQUIRED by the Shadow API. The "clientToken" may be anything, as
 * long as it's unique. This demo uses "token-" suffixed with the RTOS tick count
 * at the time the JSON document is generated. */
#define renesasDemoREPORT_JSON      \
    "{"                             \
    "\"state\":{"                   \
    "\"reported\":{"                \
	"\"temperature\": %.2f,"        \
	"\"light\": %.2f,"              \
	"\"humidity\": %.2f,"			\
	"\"pressure\": %.2f,"           \
	"\"accel\": {" 					\
		"\"x\": %.2f, \"y\": %.2f, \"z\": %.2f" \
	"},"							\
	"\"gyro\": {"					\
		"\"x\": %.2f, \"y\": %.2f, \"z\": %.2f" \
    "}"								\
    "}},"                           \
    "\"clientToken\": \"token-%d\"" \
    "}"

#define renesasDemoDESIRED_JSON      \
    "{"                             \
    "\"state\":{"                   \
    "\"desired\":{"                 \
    "\"%s\":\"%s\""                 \
    "}"                             \
    "},"                            \
    "\"clientToken\": \"token-%d\"" \
    "}"

/* Maximum amount of time a Shadow function call may block. */
#define renesasDemoTIMEOUT                    pdMS_TO_TICKS( 30000UL )

/* Max size for the name of tasks generated for Shadow. */
#define renesasDemoCHAR_TASK_NAME_MAX_SIZE    15

/* How often the "desired" state of the bulb should be changed. */
#define renesasDemoSEND_UPDATE_MS             pdMS_TO_TICKS( ( 500UL ) )

/* Maximum size of update JSON documents. */
#define renesasDemoBUFFER_LENGTH              512

/* Stack size for task that handles shadow delta and updates. */
#define renesasDemoUPDATE_TASK_STACK_SIZE     ( ( uint16_t ) configMINIMAL_STACK_SIZE * ( uint16_t ) 5 )

/* Maximum number of jsmn tokens. */
#define renesasDemoMAX_TOKENS                 40

/* Queue configuration parameters. */
#define renesasDemoSEND_QUEUE_WAIT_TICKS      3000
#define renesasDemoRECV_QUEUE_WAIT_TICKS      500
#define renesasDemoUPDATE_QUEUE_LENGTH        democonfigSHADOW_DEMO_NUM_TASKS * 2

/* The maximum amount of time tasks will wait for their updates to process.
 * Tasks should not continue executing until their updates have processed.*/
#define renesasDemoNOTIFY_WAIT_MS             pdMS_TO_TICKS( ( 100000UL ) )

extern const rx_provisioning_info *rx_provisioning_data;

/* An element of the Shadow update queue. */
typedef struct
{
    TaskHandle_t xTaskToNotify;
    uint32_t ulDataLength;
    char pcUpdateBuffer[ renesasDemoBUFFER_LENGTH ];
} ShadowQueueData_t;

/* The parameters of the Shadow tasks. */
typedef struct
{
    TaskHandle_t xTaskHandle;
    char cTaskName[ renesasDemoCHAR_TASK_NAME_MAX_SIZE ];
} ShadowTaskParam_t;

/* Shadow demo tasks. */
static void prvShadowInitTask( void * pvParameters );
static void prvChangeDesiredTask( void * pvParameters );
static void prvUpdateQueueTask( void * pvParameters );

/* Creates Shadow client and connects to MQTT broker. */
static ShadowReturnCode_t prvShadowClientCreateConnect( void );

/* Called when there's a difference between "reported" and "desired" in Shadow document. */
static BaseType_t prvDeltaCallback( void * pvUserData,
                                    const char * const pcThingName,
                                    const char * const pcDeltaDocument,
                                    uint32_t ulDocumentLength,
                                    MQTTBufferHandle_t xBuffer );

/* JSON functions. */
static uint32_t prvGenerateReportedJSON( ShadowQueueData_t * const pxShadowQueueData,
                                         const char * const pcReportedData,
                                         uint32_t ulReportedDataLength );
static BaseType_t prvIsStringEqual( const char * const pcJson,     /*lint !e971 can use char without signed/unsigned. */
                                    const jsmntok_t * const pxTok,
                                    const char * const pcString ); /*lint !e971 can use char without signed/unsigned. */

/* The update queue's handle, data structure, and memory. */
static QueueHandle_t xUpdateQueue = NULL;
static StaticQueue_t xStaticQueue;
static uint8_t ucQueueStorageArea[ renesasDemoUPDATE_QUEUE_LENGTH * sizeof( ShadowQueueData_t ) ];

/* The handle of the Shadow client shared across all tasks. */
static ShadowClientHandle_t xClientHandle;
/* Memory allocated to store the Shadow task params. */
static ShadowTaskParam_t xShadowTaskParamBuffer[ democonfigSHADOW_DEMO_NUM_TASKS ];

/*-----------------------------------------------------------*/

static uint32_t prvGenerateReportedJSON( ShadowQueueData_t * const pxShadowQueueData,
                                         const char * const pcReportedData,
                                         uint32_t ulReportedDataLength )
{
	rx_sensor_data data;
	read_sensor(&data);

    return ( uint32_t ) snprintf( ( char * ) ( pxShadowQueueData->pcUpdateBuffer ),
                                  renesasDemoBUFFER_LENGTH,
                                  renesasDemoREPORT_JSON,
								  data.temperature,
								  data.als,
								  data.humidity,
								  data.pressure,
								  data.accel.xacc,
								  data.accel.yacc,
								  data.accel.zacc,
								  data.gyro.xacc,
								  data.gyro.yacc,
								  data.gyro.zacc,
                                  ( int ) xTaskGetTickCount() );
}

/*-----------------------------------------------------------*/

static BaseType_t prvIsStringEqual( const char * const pcJson,    /*lint !e971 can use char without signed/unsigned. */
                                    const jsmntok_t * const pxTok,
                                    const char * const pcString ) /*lint !e971 can use char without signed/unsigned. */
{
    uint32_t ulStringSize = ( uint32_t ) pxTok->end - ( uint32_t ) pxTok->start;
    BaseType_t xStatus = pdFALSE;

    if( pxTok->type == JSMN_STRING )
    {
        if( ( pcString[ ulStringSize ] == 0 ) &&
            ( strncmp( pcJson + pxTok->start, pcString, ulStringSize ) == 0 ) )
        {
            xStatus = pdTRUE;
        }
    }

    return xStatus;
}

/*-----------------------------------------------------------*/

static BaseType_t prvDeltaCallback( void * pvUserData,
                                    const char * const pcThingName,
                                    const char * const pcDeltaDocument,
                                    uint32_t ulDocumentLength,
                                    MQTTBufferHandle_t xBuffer )
{
    int32_t lNbTokens;
    uint16_t usTokenIndex;
    uint32_t ulStringSize;
    jsmn_parser xJSMNParser;
    ShadowQueueData_t xShadowQueueData;
    jsmntok_t pxJSMNTokens[ renesasDemoMAX_TOKENS ];

    /* Silence compiler warnings about unused variables. */
    ( void ) pvUserData;
    ( void ) xBuffer;
    ( void ) pcThingName;

    jsmn_init( &xJSMNParser );
    memset( &xShadowQueueData, 0x00, sizeof( ShadowQueueData_t ) );

    lNbTokens = ( int32_t ) jsmn_parse( &xJSMNParser,
                                        pcDeltaDocument,
                                        ( size_t ) ulDocumentLength,
                                        pxJSMNTokens,
                                        ( unsigned int ) renesasDemoMAX_TOKENS );

    if( lNbTokens > 0 )
    {
        /* Find the new reported state. */
        for( usTokenIndex = 0; usTokenIndex < ( uint16_t ) lNbTokens; usTokenIndex++ )
        {
            if( prvIsStringEqual( pcDeltaDocument, &pxJSMNTokens[ usTokenIndex ], "state" ) == pdTRUE )
            {
                ulStringSize = pxJSMNTokens[ usTokenIndex + ( uint16_t ) 1 ].end
                               - pxJSMNTokens[ usTokenIndex + ( uint16_t ) 1 ].start;
                configASSERT( ulStringSize < renesasDemoBUFFER_LENGTH );

                /* Generate a new JSON document with new reported state. */
                xShadowQueueData.ulDataLength = prvGenerateReportedJSON( &xShadowQueueData,
                                                                         &pcDeltaDocument[ pxJSMNTokens[ usTokenIndex + ( uint16_t ) 1 ].start ],
                                                                         ulStringSize );

                /* Add new reported state to update queue. */
                if( xQueueSendToBack( xUpdateQueue, &xShadowQueueData, renesasDemoSEND_QUEUE_WAIT_TICKS ) == pdTRUE )
                {
                    configPRINTF( ( "Successfully added new reported state to update queue.\r\n" ) );
                }
                else
                {
                    configPRINTF( ( "Update queue full, deferring reported state update.\r\n" ) );
                }

                break;
            }
        }
    }

    return pdFALSE;
}

/*-----------------------------------------------------------*/

static void prvUpdateQueueTask( void * pvParameters )
{
    ShadowReturnCode_t xReturn;
    ShadowOperationParams_t xUpdateParams;
    ShadowQueueData_t xShadowQueueData;

    ( void ) pvParameters;

    xUpdateParams.pcThingName = rx_provisioning_data->device_name;
    xUpdateParams.xQoS = eMQTTQoS0;
    xUpdateParams.pcData = xShadowQueueData.pcUpdateBuffer;
    /* Keep subscriptions across multiple calls to SHADOW_Update. */
    xUpdateParams.ucKeepSubscriptions = pdTRUE;

    for( ; ; )
    {
        if( xQueueReceive( xUpdateQueue, &xShadowQueueData, renesasDemoRECV_QUEUE_WAIT_TICKS ) == pdTRUE )
        {
             configPRINTF( ( "Performing Thing Shadow update.\r\n" ) );
             xUpdateParams.ulDataLength = xShadowQueueData.ulDataLength;

             xReturn = SHADOW_Update( xClientHandle, &xUpdateParams, renesasDemoTIMEOUT );

             if( xReturn == eShadowSuccess )
             {
                 configPRINTF( ( "Successfully performed update.\r\n" ) );
             }
             else
             {
                 configPRINTF( ( "Update failed, returned %d.\r\n", xReturn ) );
             }

             /* Notify tasks that their update was completed. */
             if( xShadowQueueData.xTaskToNotify != NULL )
             {
                 xTaskNotifyGive( xShadowQueueData.xTaskToNotify );
             }
         }
    }
}
/*-----------------------------------------------------------*/

static ShadowReturnCode_t prvShadowClientCreateConnect( void )
{
    MQTTAgentConnectParams_t xConnectParams;
    ShadowCreateParams_t xCreateParams;
    ShadowReturnCode_t xReturn;

    xCreateParams.xMQTTClientType = eDedicatedMQTTClient;
    xReturn = SHADOW_ClientCreate( &xClientHandle, &xCreateParams );

    if( xReturn == eShadowSuccess )
    {
        xConnectParams.pcURL = rx_provisioning_data->endpoint;
    	//xConnectParams.pcURL = "mimolette.zvuk.net";
        xConnectParams.usPort = clientcredentialMQTT_BROKER_PORT;

        xConnectParams.xFlags = democonfigMQTT_AGENT_CONNECT_FLAGS;
        xConnectParams.pcCertificate = NULL;
        xConnectParams.ulCertificateSize = 0;
        xConnectParams.pxCallback = NULL;
        xConnectParams.pvUserData = &xClientHandle;

        xConnectParams.pucClientId = (uint8_t *) rx_provisioning_data->device_name;
        xConnectParams.usClientIdLength = ( uint16_t ) rx_provisioning_data->device_name_len;

        xReturn = SHADOW_ClientConnect( xClientHandle,
                                        &xConnectParams,
                                        renesasDemoTIMEOUT );

        if( xReturn != eShadowSuccess )
        {
            configPRINTF( ( "Shadow_ClientConnect unsuccessful, returned %d.\r\n", xReturn ) );
        }
    }
    else
    {
        configPRINTF( ( "Shadow_ClientCreate unsuccessful, returned %d.\r\n", xReturn ) );
    }

    return xReturn;
}
/*-----------------------------------------------------------*/

static void prvChangeDesiredTask( void * pvParameters )
{
    uint8_t ucBulbState = 0;
    uint32_t ulInitialReportLength;
    TickType_t xLastWakeTime;
    ShadowTaskParam_t * pxShadowTaskParam;
    ShadowQueueData_t xShadowQueueData;
    char pcInitialReportBuffer[ renesasDemoINITIAL_REPORT_BUFFER_LENGTH ];

    /* Initialize parameters. */
    pxShadowTaskParam = ( ShadowTaskParam_t * ) pvParameters; /*lint !e9087 Safe cast from context. */
    memset( &xShadowQueueData, 0x00, sizeof( ShadowQueueData_t ) );
    xShadowQueueData.xTaskToNotify = pxShadowTaskParam->xTaskHandle;

    /* Add the initial state to the update queue, wait for the update to complete. */
    ulInitialReportLength = snprintf( pcInitialReportBuffer,
                                      renesasDemoINITIAL_REPORT_BUFFER_LENGTH,
                                      renesasDemoINITIAL_REPORT_FORMAT,
                                      pxShadowTaskParam->cTaskName );
    xShadowQueueData.ulDataLength = prvGenerateReportedJSON( &xShadowQueueData,
                                                             pcInitialReportBuffer,
                                                             ulInitialReportLength );
    /* The calls below should never fail because the queue should be empty currently. */
    //configASSERT( xQueueSendToBack( xUpdateQueue, &xShadowQueueData, renesasDemoSEND_QUEUE_WAIT_TICKS ) == pdTRUE );
    //configASSERT( ulTaskNotifyTake( pdTRUE, renesasDemoNOTIFY_WAIT_MS ) == 1 );

    xLastWakeTime = xTaskGetTickCount();

    /* Keep changing the desired state of light bulb periodically. */
    for( ; ; )
    {
        configPRINTF( ( "%s changing reported state.\r\n", pxShadowTaskParam->cTaskName ) );

        /* Toggle the desired state and generate a new JSON document. */
        xShadowQueueData.ulDataLength = prvGenerateReportedJSON( &xShadowQueueData,
                                                                pxShadowTaskParam->cTaskName,
                                                                ucBulbState );

        /* Add the new desired state to the update queue. */
        if( xQueueSendToBack( xUpdateQueue, &xShadowQueueData, renesasDemoSEND_QUEUE_WAIT_TICKS ) == pdTRUE )
        {
            /* If the new desired state was successfully added, wait for notification that the update completed. */
            configASSERT( ulTaskNotifyTake( pdTRUE, renesasDemoNOTIFY_WAIT_MS ) == 1 );
            configPRINTF( ( "%s done changing reported state.\r\n", pxShadowTaskParam->cTaskName ) );
        }
        else
        {
            configPRINTF( ( "Update queue full, deferring desired state change.\r\n" ) );
        }

        vTaskDelay(renesasDemoSEND_UPDATE_MS );
    }
}
/*-----------------------------------------------------------*/

static void prvShadowInitTask( void * pvParameters )
{
    uint8_t ucTask;
    ShadowReturnCode_t xReturn;
    ShadowOperationParams_t xOperationParams;
    ShadowCallbackParams_t xCallbackParams;

    ( void ) pvParameters;

	init_sensors();


    vTaskDelay(10000);	// todo: this is renesas issue.


    /* Initialize the update queue and Shadow client; set all pending updates to false. */
    xUpdateQueue = xQueueCreateStatic( renesasDemoUPDATE_QUEUE_LENGTH,
                                       sizeof( ShadowQueueData_t ),
                                       ucQueueStorageArea,
                                       &xStaticQueue );
    xReturn = prvShadowClientCreateConnect();

    if( xReturn == eShadowSuccess )
    {
        xOperationParams.pcThingName = rx_provisioning_data->device_name;
        xOperationParams.xQoS = eMQTTQoS0;
        xOperationParams.pcData = NULL;
        /* Don't keep subscriptions, since SHADOW_Delete is only called here once. */
        xOperationParams.ucKeepSubscriptions = pdFALSE;

        /* Delete any previous shadow. */
        xReturn = SHADOW_Delete( xClientHandle,
                                 &xOperationParams,
                                 renesasDemoTIMEOUT );

        /* Atttempting to delete a non-existant shadow returns eShadowRejectedNotFound.
         * Either eShadowSuccess or eShadowRejectedNotFound signify that there's no
         * existing Thing Shadow, so both values are ok. */
        if( ( xReturn == eShadowSuccess ) || ( xReturn == eShadowRejectedNotFound ) )
        {
            /* Register callbacks. This demo doesn't use updated or deleted callbacks, so
             * those members are set to NULL. The callbacks are registered after deleting
             * the Shadow so that any previous Shadow doesn't unintentionally trigger the
             * delta callback.*/
            xCallbackParams.pcThingName = rx_provisioning_data->device_name;
            xCallbackParams.xShadowUpdatedCallback = NULL;
            xCallbackParams.xShadowDeletedCallback = NULL;
            xCallbackParams.xShadowDeltaCallback = prvDeltaCallback;

            xReturn = SHADOW_RegisterCallbacks( xClientHandle,
                                                &xCallbackParams,
                                                renesasDemoTIMEOUT );
        }
    }

    if( xReturn == eShadowSuccess )
    {
        configPRINTF( ( "Shadow client initialized.\r\n" ) );

        /* Create the update task which will process the update queue. */
        ( void ) xTaskCreate( prvUpdateQueueTask,
                              renesasDemoUPDATE_TASK_NAME,
                              renesasDemoUPDATE_TASK_STACK_SIZE,
                              NULL,
                              democonfigSHADOW_DEMO_TASK_PRIORITY,
                              NULL );


		( void ) snprintf( ( char * ) ( &( xShadowTaskParamBuffer[ ucTask ] ) )->cTaskName,
						   renesasDemoCHAR_TASK_NAME_MAX_SIZE,
						   renesasDemoCHAR_TASK_NAME,
						   ucTask );
		( void ) xTaskCreate( prvChangeDesiredTask,
							  ( const char * ) ( &( xShadowTaskParamBuffer[ ucTask ] ) )->cTaskName,
							  democonfigSHADOW_DEMO_TASK_STACK_SIZE,
							  &( xShadowTaskParamBuffer[ ucTask ] ),
							  democonfigSHADOW_DEMO_TASK_PRIORITY,
							  &( ( xShadowTaskParamBuffer[ ucTask ] ).xTaskHandle ) );

    }
    else
    {
        configPRINTF( ( "Failed to initialize Shadow client.\r\n" ) );
    }

    vTaskDelete( NULL );
}

/* Create the shadow demo main task which will act as a client application to
 * request periodic change in state (color) of light bulb.  */
void vStartRenesasDemoTask( void )
{
    ( void ) xTaskCreate( prvShadowInitTask,
                          "MainDemoTask",
                          renesasDemoUPDATE_TASK_STACK_SIZE,
                          NULL,
                          tskIDLE_PRIORITY,
                          NULL );
}
/*-----------------------------------------------------------*/
