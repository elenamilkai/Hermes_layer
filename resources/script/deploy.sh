# Script to spread built package to all cluster nodes
# Note: set resources/config/cluster_ips first

# import util
util_path=$(dirname "$0")"/util.sh"
source "$util_path"

# 1. organize executables, resources and required libraries
rm -rf "$deploy_dir"
echo "Copying built files..."
mkdir -p "$deploy_dir"

# executables
exe_dir="$build_dir"/"$exe_dir_name"
exe_deploy_dir="$deploy_dir"/"$exe_dir_name"
cp -r "$exe_dir"/ "$exe_deploy_dir"/

# calcite
calcite_jar_path="$root_dir"/"$calcite_dir_name""/target/""$calcite_jar_name"
calcite_deploy_jar_path="$deploy_dir"/"$calcite_dir_name""/target/""$calcite_jar_name"
mkdir -p "$(dirname "${calcite_deploy_jar_path}")"
cp -r "$calcite_jar_path" "$calcite_deploy_jar_path"

# resources
resource_deploy_dir="$deploy_dir""/resources/"
mkdir -p "$(dirname "${resource_deploy_dir}")"
cp -r "$resource_dir"/ "$resource_deploy_dir"/

# libs
lib_names=("aws-cpp-sdk_ep" "caf_ep" "graphviz_ep")
lib_suffix="install/lib"
lib_root_dir="$build_dir""/_deps"
lib_deploy_root_dir="$deploy_dir""/libs"

for lib_name in "${lib_names[@]}"
do
  lib_dir="$lib_root_dir"/"$lib_name"/"$lib_suffix"
  lib_deploy_dir="$lib_deploy_root_dir"/"$lib_name"/"$lib_suffix"
  mkdir -p "$(dirname "${lib_deploy_dir}")"
  cp -r "$lib_dir"/ "$lib_deploy_dir"/
done

# create a temp directory
mkdir -p "$temp_deploy_dir"

echo -e "done\n"

# 2. deploy organized package for each node
echo "Sending built files to cluster nodes..."

for slave_ip in "${slave_ips[@]}"
do
  echo -n "  Sending to ""$slave_ip""... "
  check_or_add_to_known_hosts "$slave_ip"
  run_command "$pem_path" "$slave_ip" rm -rf "$deploy_dir"
  scp -rqi "$pem_path" "$deploy_dir"/ ubuntu@"$slave_ip":"$deploy_dir"/
  echo "  done"
done

echo "done"
