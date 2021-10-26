#ifndef _LATLONGCONV_H_
#define _LATLONGCONV_H_

void LLtoUTM(int ReferenceEllipsoid, double Lat, double Long, 
     double *UTMNorthing, double *UTMEasting, char* UTMZone);

void UTMtoLL(int ReferenceEllipsoid, double UTMNorthing, double UTMEasting, char* UTMZone,
     double* Lat,  double* Long );

char UTMLetterDesignator(double Lat);

#if 0
void LLtoSwissGrid(double Lat, double Long, 
   double *SwissNorthing, double *SwissEasting);

void SwissGridtoLL(double SwissNorthing, double SwissEasting, 
   double* Lat, double* Long);
#endif

#if 0
class Ellipsoid
{
public:
  Ellipsoid(){};
  Ellipsoid(int Id, char* name, double radius, double ecc)
  {
    id = Id; ellipsoidName = name; 
    EquatorialRadius = radius; eccentricitySquared = ecc;
  }
  
  int id;
  char* ellipsoidName;
  double EquatorialRadius; 
  double eccentricitySquared;  
  
};
#else
typedef struct {
  int id;
  char* ellipsoidName;
  double EquatorialRadius; 
  double eccentricitySquared;  
} Ellipsoid ;
#endif


#endif
