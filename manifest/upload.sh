#!/bin/sh -x

FORCE=${1}

git push origin ${FORCE}master:master&
git push origin ${FORCE}warrior:warrior&
git push origin ${FORCE}zeus:zeus&
git push origin ${FORCE}dunfell:dunfell&
git push origin ${FORCE}master-next:master-next&
git push origin ${FORCE}warrior-next:warrior-next&
git push origin ${FORCE}zeus-next:zeus-next&
git push origin ${FORCE}dunfell-next:dunfell-next&
wait
