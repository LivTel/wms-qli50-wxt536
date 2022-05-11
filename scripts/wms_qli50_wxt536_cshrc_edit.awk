#!/bin/awk -f
BEGIN{
  copy = 1
  nextcopy = 0
} 
/.*\<wms_qli50_wxt536_install:start\>.*/ { copy = 0 }
/.*\<wms_qli50_wxt536_install:end\>.*/ { nextcopy = 1 }
 { 
   if(copy == 1) {
     print $0
   }
   if(nextcopy == 1) {
     nextcopy = 0
     copy = 1 
   }
}
