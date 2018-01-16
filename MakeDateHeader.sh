#!/bin/bash
echo "#define __CK_BUILD_DATE ""\""$(date "+%F-%T")"\"" > "../../DateHeader.h"
temp=$(git log --pretty=format:'%h' -n 1)
echo "#define __CK_COMMIT_ID " ""\"$temp""\" >> "../../DateHeader.h"