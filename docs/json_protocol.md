# Hardware test data protocal

# Hardware Test

* ajax request json data for test
  ```json
  {
      "categories": "hardware_test", 
      "type": "test"
  }
  ```
* response ajax json data for test
  ```json
  {
      "categories": "hardware_test",
      "type": "test",
      "data": {
          "eeprom":{
              "status":"error"
          },
          "gpio":{
              "status":"ok"
          },
          "input":{
              "status":"error"
          },
          "rtc":{
              "status":"error"
          },
          "tmp75":{
              "status":"error"
          },
          "udisk":{
              "status":"error"
          },
          "us100":{
              "status":"error"
          }
      },
      "status":"ok"
  }
  ```
