# start server on the current node

# import util
util_path=$(dirname "$0")"/util.sh"
source "$util_path"

# export lib paths to LD_LIBRARY_PATH
lib_deploy_dir="$deploy_dir""/libs"
export LD_LIBRARY_PATH=${lib_deploy_dir}/aws-cpp-sdk_ep/install/lib:${lib_deploy_dir}/caf_ep/install/lib:\
${lib_deploy_dir}/graphviz_ep/install/lib:${lib_deploy_dir}/graphviz_ep/install/lib/graphviz

# start server
server_exe_name="fpdb-main-server"
server_deploy_dir="$deploy_dir"/"$exe_dir_name"
server_pid_path="$temp_deploy_dir"/"$server_pid_name"

pushd "$(dirname "$0")" > /dev/null
cd "$server_deploy_dir"
nohup "./""$server_exe_name" >/dev/null 2>&1 &
echo $! > "$server_pid_path"
popd > /dev/null
