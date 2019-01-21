<?php 

namespace Snowflake;

class Client
{
	/**
	 * The current host
	 *
	 * @var string
	 */
	protected $host = null;
	
	/**
	 * The current port
	 *
	 * @var int
	 */
	protected $port = null;

	/**
	 * The binded socket
	 * 
	 * @var resource
	 */
	protected $socket = null;

	/**
	 * Connection status
	 *
	 * @var bool
	 */
	private $isConnected = false;

	/**
	 * Construct a new client
	 *
	 * @param string 			$hostname
	 * @param int 				$port
	 */
	public function __construct(string $hostname = '127.0.0.1', int $port = 8008)
	{
		$this->host = $hostname;
		$this->port = $port;
		
		// create a socket
		$this->createSocket();

		// connect
		$this->connect();
	}

	/**
	 * Close the connection on desctruct
	 */
	public function __destruct()
	{
		if ($this->isConnected) {
			socket_close($this->socket);
		}
	}

	/**
	 *  Create new socket resource 
	 *
	 * @return void
	 */
	protected function createSocket()
	{
		$this->socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);

		if ($this->socket === false) {
		    throw new ConnectionException('Could not create socket: ' . socket_strerror(socket_last_error()));
		}
	}

	/**
	 * Connect
	 *
	 * @return void
	 */
	protected function connect()
	{
		if (socket_connect($this->socket, $this->host, $this->port) === false) {
		    echo "Could not connect to Snowflake server: " . socket_strerror(socket_last_error($this->socket)) . "\n";
		}

		$this->isConnected = true;
	}

	/**
	 * Send a command and return the response
	 *
	 * @param string $command
	 * @return string The response from snowflake
	 */
	protected function sendCommand(string $command) : string
	{
		socket_write($this->socket, $command, strlen($command));

		$buffer = "";
		while ($r = trim(socket_read($this->socket, 1024, PHP_NORMAL_READ))) {
		    $buffer .= $r . "\n";
		}

		$buffer = trim($buffer);

		// remove the first character
		if ($buffer[0] === '-') {
			throw new ErrorException('Snowflake response contained an error.');
		}

		return substr($buffer, 1);
	}

	/**
	 * Fetch & parse info from the snowflake server
	 *
	 * @return array
	 */
	public function info() : array
	{
		$info = [];
		foreach(explode("\n", $this->sendCommand("INFO")) as $line)
		{
			$p = strpos($line, ':');
			$info[substr($line, 0, $p)] = substr($line, $p + 1);
		}

		return $info;
	}

	/**
	 * Fetches a id from the snowflake server
	 *
	 * @return int
	 */
	public function id() : int
	{
		return (int) $this->sendCommand('GET');
	}
}