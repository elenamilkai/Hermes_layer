# stop the system on all cluster nodes

# import util
util_path=$(dirname "$0")"/util.sh"
source "$util_path"

# stop calcite server on master
echo "Stopping calcite server on master node..."

calcite_pid_path="$temp_deploy_dir"/"$calcite_pid_name"
if [ -e "$calcite_pid_path" ]; then
	kill "$(cat "$calcite_pid_path")"
	rm "$calcite_pid_path"
fi

echo "done"

# stop server on each slave node
echo "Stopping server on slave nodes..."

for slave_ip in "${slave_ips[@]}"
do
  echo -n "  Stopping ""$slave_ip""... "
  check_or_add_to_known_hosts "$slave_ip"
  run_command "$pem_path" "$slave_ip" "$deploy_dir""/resources/script/stop_node.sh"
  echo "  done"
done

echo "done"
