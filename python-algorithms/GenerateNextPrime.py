# When the plugin development is finished, you can copy the associated Python file 
# to C:/Users/Janos/.Tulip-5.3/plugins/python
# or D:/porgydev/porgy_install/bin/../lib/tulip/python/
# and it will be automatically loaded at Tulip startup

from tulip import tlp
import tulipplugins
#import logging

class AugmentationGenerateNextPrime(tlp.IntegerAlgorithm):
  def __init__(self, context):
    tlp.IntegerAlgorithm.__init__(self, context)
    self.addIntegerPropertyParameter("result", "", "UID", True, False, True)
    #logging.basicConfig(filename='d:/Profiles/Janos/Dropbox/KCL/Porgy/pylog.txt', level=logging.DEBUG)

  def check(self):
    return (True, "")
    
  def run(self):
    RelDbType = self.graph.getStringProperty("RelDbType")
    UID = self.graph.getIntegerProperty("UID")
    IsGeneratedByTC = self.graph.getIntegerProperty("IsGeneratedByTC")
    IsTrivial = self.graph.getIntegerProperty("IsTrivial")
    max_prime_uid = 0

    for n in self.graph.getNodes():
      if RelDbType[n] == "FD" and IsGeneratedByTC[n] == 0 and UID[n] > max_prime_uid:
        max_prime_uid = UID[n]
        
    for n in self.graph.getNodes():
      if RelDbType[n] == "FD":
        if UID[n] == -1:
          if IsTrivial[n] == 1:
            #logging.debug('IsTrivial==1')
            self.result[n] = 1
          else:
            #logging.debug('IsTrivial<>1')
            next_uid = max_prime_uid+1
            #Bertrand's postulate: there exists a prime between Pn and 2*Pn where Pn is the n-th prime            
            for possiblePrime in range(next_uid, 2 * next_uid):
              isPrime = True # Assume next_uid is prime until shown it is not.             
              for num in range(2, int(possiblePrime ** 0.5) + 1):
                if possiblePrime % num == 0:
                  isPrime = False
                  break              
              if isPrime:
                next_uid = possiblePrime
                break
            #logging.debug(next_uid)
            self.result[n] = next_uid
            max_prime_uid = next_uid
        else:
          #logging.debug('UID<>-1')
          self.result[n] = UID[n]
          
    return True
   
tulipplugins.registerPluginOfGroup("AugmentationGenerateNextPrime", "Generate Next Prime UID", "Janos Varga", "19/01/2019", "", "1.0", "PORGY")
