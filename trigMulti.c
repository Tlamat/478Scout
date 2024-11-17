long trigMulti(long int inputVal, long int theta, char trig, signed int xMultiplier, signed int yMultiplier) {
   
   long newVal;
   
   if(theta == 0) {
      if(trig == 'c') {
         newVal = inputVal * cos0 / 10000 * yMultiplier;
      } else if(trig == 's') {
         newVal = inputVal * sin0 / 10000 * xMultiplier;
      }
   } else if(theta == 7) {
      if(trig == 'c') {
         newVal = inputVal * cos7 / 10000 * yMultiplier;
      } else if(trig == 's') {
         newVal = inputVal * sin7 / 10000 * xMultiplier;
      }
   } else if(theta == 15) {
      if(trig == 'c') {
         newVal = inputVal * cos15 / 10000 * yMultiplier;
      } else if(trig == 's') {
         newVal = inputVal * sin15 / 10000 * xMultiplier;
      }
   } else if(theta == 22) {
      if(trig == 'c') {
         newVal = inputVal * cos22 / 10000 * yMultiplier;
      } else if(trig == 's') {
         newVal = inputVal * sin22 / 10000 * xMultiplier;
      }
   } else if(theta == 30) {
      if(trig == 'c') {
         newVal = inputVal * cos30 / 10000 * yMultiplier;
      } else if(trig == 's') {
         newVal = inputVal * sin30 / 10000 * xMultiplier;
      }
   } else if(theta == 37) {
      if(trig == 'c') {
         newVal = inputVal * cos37 / 10000 * yMultiplier;
      } else if(trig == 's') {
         newVal = inputVal * sin37 / 10000 * xMultiplier;
      }
   } else if(theta == 45) {
      if(trig == 'c') {
         newVal = inputVal * cos45 / 10000 * yMultiplier;
      } else if(trig == 's') {
         newVal = inputVal * sin45 / 10000 * xMultiplier;
      }
   } else if(theta == 52) {
      if(trig == 'c') {
         newVal = inputVal * cos52 / 10000 * yMultiplier;
      } else if(trig == 's') {
         newVal = inputVal * sin52 / 10000 * xMultiplier;
      }
      
   } else if(theta == 60) {
      if(trig == 'c') {
         newVal = inputVal * cos60 / 10000 * yMultiplier;
      } else if(trig == 's') {
         newVal = inputVal * sin60 / 10000 * xMultiplier;
      }
   } else if(theta == 67) {
      if(trig == 'c') {
         newVal = inputVal * cos67 / 10000 * yMultiplier;
      } else if(trig == 's') {
         newVal = inputVal * sin67 / 10000 * xMultiplier;
      }
      
   } else if(theta == 75) {
      if(trig == 'c') {
         newVal = inputVal * cos75 / 10000 * yMultiplier;
      } else if(trig == 's') {
         newVal = inputVal * sin75 / 10000 * xMultiplier;
      }
   } else if(theta == 82) {
      if(trig == 'c') {
         newVal = inputVal * cos82 / 10000 * yMultiplier;
      } else if(trig == 's') {
         newVal = inputVal * sin82 / 10000 * xMultiplier;
      }
      
   } else if(theta == 90) {
      if(trig == 'c') {
         newVal = inputVal * cos90 / 10000 * yMultiplier;
      } else if(trig == 's') {
         newVal = inputVal * sin90 / 10000 * xMultiplier;
      }
   }
   return newVal;
}
