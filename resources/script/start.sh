# start the system on all cluster nodes

# import util
util_path=$(dirname "$0")"/util.sh"
source "$util_path"

# start calcite server on master
echo "Starting calcite server on master node..."

calcite_jar_path="$deploy_dir"/"$calcite_dir_name""/target/""$calcite_jar_name"
calcite_pid_path="$temp_deploy_dir"/"$calcite_pid_name"
java -jar "$calcite_jar_path" &
echo $! > "$calcite_pid_path"

echo "done"

# start server on each slave node
echo "Starting server on slave nodes..."

for slave_ip in "${slave_ips[@]}"
do
  echo -n "  Starting ""$slave_ip""... "
  check_or_add_to_known_hosts "$slave_ip"
  run_command "$pem_path" "$slave_ip" "$deploy_dir""/resources/script/start_node.sh"
  echo "  done"
done

echo "done"
