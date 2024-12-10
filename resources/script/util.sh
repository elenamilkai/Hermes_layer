
# configurable parameters
export install_dependency=false
export clean=false
export build_parallel=8
export build_dir_name="build"
export deploy_dir_name="FPDB-build"
export temp_dir_name="temp"
export pem_path="$HOME""/.aws/yifei-aws-wisc.pem"

# fixed parameters
export targets=("fpdb-main-server" "fpdb-main-test")
export exe_dir_name="fpdb-main"
export calcite_jar_name="flexpushdowndb.thrift.calcite-1.0-SNAPSHOT.jar"
export calcite_dir_name="fpdb-calcite/java"
export server_pid_name="FPDB-server.pid"
export calcite_pid_name="calcite-server.pid"

# get script path
pushd "$(dirname "$0")" > /dev/null
script_dir=$(pwd)
popd > /dev/null

# directories
resource_dir="$(dirname "${script_dir}")"
root_dir="$(dirname "${resource_dir}")"
build_dir="${root_dir}"/"${build_dir_name}"
deploy_dir=$HOME/$deploy_dir_name
temp_deploy_dir="$deploy_dir"/"$temp_dir_name"
export script_dir resource_dir root_dir build_dir deploy_dir temp_deploy_dir

# slave ips
master_ip="$(curl -s ifconfig.me)"
cluster_ips_path="${resource_dir}""/config/cluster_ips"
while IFS= read -r line || [[ -n "$line" ]];
do
  if [ "$line" != "$master_ip" ]; then
    slave_ips+=("$line")
  fi
done < "$cluster_ips_path"
export slave_ips

# catch ctrl_c
trap ctrl_c INT
function ctrl_c() {
  echo "*** Trapped CTRL-C, exit"
  popd > /dev/null
  exit 0
}

# check if ip is added to ~/.ssh/known_hosts, if not, add first
function check_or_add_to_known_hosts() {
  ip=$1
  SSHKey=$(ssh-keygen -F "$ip")
  if [ -z "$SSHKey" ]; then
    SSHKey=$(ssh-keyscan -H "$ip" 2> /dev/null)
    echo "$SSHKey" >> ~/.ssh/known_hosts
  fi
}

# run command remotely
function run_command() {
  pem_path=$1
  ip=$2
  shift 2
  ssh -i "$pem_path" ubuntu@"$ip" "$@"
}
