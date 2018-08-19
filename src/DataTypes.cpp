#include "DatatypeBase.hpp"

int addDatatypes()
{
DatatypeBase Integer;
Integer.setID("int");
Integer.addOperator(Add);
Integer.addOperator(Sub);
Integer.addOperator(Mul);
Integer.addOperator(Div);
Integer.addOperator(Equal);
Integer.addOperator(AEqual);
Integer.addOperator(SEqual);
Integer.addOperator(MEqual);
Integer.addOperator(DEqual);
Integer.addOperator(GTHAN);
Integer.addOperator(LTHAN);
Integer.addOperator(LEQUALTHAN);
Integer.addOperator(GEQUALTHAN);
Integer.addTypeValue("int");
Integer.addTypeValue("float");


DatatypeBase Float;
Float.setID("float");
Float.addOperator(Add);
Float.addOperator(Sub);
Float.addOperator(Mul);
Float.addOperator(Div);
Float.addOperator(Equal);
Float.addOperator(AEqual);
Float.addOperator(SEqual);
Float.addOperator(MEqual);
Float.addOperator(DEqual);
Float.addOperator(GTHAN);
Float.addOperator(LTHAN);
Float.addOperator(LEQUALTHAN);
Float.addOperator(GEQUALTHAN);
Float.addTypeValue("int");
Float.addTypeValue("float");

return 0;
}
