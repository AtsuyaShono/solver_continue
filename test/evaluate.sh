#!/bin/sh

echo "
synopsys01.txt
"
./evaluaterFast testcases/synopsys01.txt outputs/01_out.txt
echo "
synopsys02.txt
"
./evaluaterFast testcases/synopsys02.txt outputs/02_out.txt
echo "
synopsys03.txt
"
./evaluaterFast testcases/synopsys03.txt outputs/03_out.txt
echo "
synopsys04.txt
"
./evaluaterFast testcases/synopsys04.txt outputs/04_out.txt
echo "
synopsys05.txt
"
./evaluaterFast testcases/synopsys05.txt outputs/05_out.txt
echo "
synopsys06.txt
"
./evaluaterFast testcases/synopsys06.txt outputs/06_out.txt
echo "
synopsyshidden01.txt
"
./evaluaterFast testcases/synopsyshidden01.txt outputs/h01_out.txt
echo "
synopsyshidden02.txt
"
./evaluaterFast testcases/synopsyshidden02.txt outputs/h02_out.txt
echo "
synopsyshidden03.txt
"
./evaluaterFast testcases/synopsyshidden03.txt outputs/h03_out.txt
