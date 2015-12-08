#include <pebble_worker.h>
#include <time.h>

#define SLAPS 0
#define TIME 0

int slaps;
int prevAccel = 0;
int prevChange = 0;
int accel = 0;
int change = 0;
time_t slapTime;

static void data_handler(AccelData *data, uint32_t num_samples) {
  accel = data[0].z;
  change = accel - prevAccel;
  
  if((change > 500) && (prevChange > -500)) {
      slaps++; 
      psleep(500);
      slapTime = time(NULL); 
  }

  prevAccel = data[0].z;
  prevChange = change;
  
  AppWorkerMessage msg_data = {.data0 =  slaps};
  AppWorkerMessage msg_data_2 = {.data1 = slapTime};
  
  app_worker_send_message(SLAPS, &msg_data);
  app_worker_send_message(TIME, &msg_data_2);
  
  prevAccel = data[0].z;
  prevChange = change;
}

static void worker_init() {
  // Subscribe to the accelerometer data service
  int num_samples = 30;
  accel_data_service_subscribe(num_samples, data_handler);
  
  // Choose update rate
  accel_service_set_sampling_rate(ACCEL_SAMPLING_100HZ);
}

static void worker_deinit() {
  // Stop using the AccelDataService
  accel_data_service_unsubscribe();
}

int main(void) {
  worker_init();
  worker_event_loop();
  worker_deinit();
}