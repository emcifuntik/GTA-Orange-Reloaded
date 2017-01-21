# Install script for directory: /mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Lib/LibStatic

# Set the install prefix
IF(NOT DEFINED CMAKE_INSTALL_PREFIX)
  SET(CMAKE_INSTALL_PREFIX "/usr/local")
ENDIF(NOT DEFINED CMAKE_INSTALL_PREFIX)
STRING(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
IF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  IF(BUILD_TYPE)
    STRING(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  ELSE(BUILD_TYPE)
    SET(CMAKE_INSTALL_CONFIG_NAME "")
  ENDIF(BUILD_TYPE)
  MESSAGE(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
ENDIF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)

# Set the component getting installed.
IF(NOT CMAKE_INSTALL_COMPONENT)
  IF(COMPONENT)
    MESSAGE(STATUS "Install component: \"${COMPONENT}\"")
    SET(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  ELSE(COMPONENT)
    SET(CMAKE_INSTALL_COMPONENT)
  ENDIF(COMPONENT)
ENDIF(NOT CMAKE_INSTALL_COMPONENT)

# Install shared libraries without execute permission?
IF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  SET(CMAKE_INSTALL_SO_NO_EXE "1")
ENDIF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Lib/RakNetLibStatic/libRakNetLibStatic.a")
  IF (CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  ENDIF (CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
  IF (CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  ENDIF (CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
FILE(INSTALL DESTINATION "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Lib/RakNetLibStatic" TYPE STATIC_LIBRARY FILES "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Lib/LibStatic/libRakNetLibStatic.a")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/AutopatcherPatchContext.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/AutopatcherRepositoryInterface.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/Base64Encoder.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/BitStream.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/CCRakNetSlidingWindow.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/CCRakNetUDT.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/CheckSum.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/CloudClient.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/CloudCommon.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/CloudServer.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/CommandParserInterface.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/ConnectionGraph2.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/ConsoleServer.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/DataCompressor.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/DirectoryDeltaTransfer.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/DR_SHA1.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/DS_BinarySearchTree.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/DS_BPlusTree.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/DS_BytePool.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/DS_ByteQueue.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/DS_Hash.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/DS_Heap.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/DS_HuffmanEncodingTree.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/DS_HuffmanEncodingTreeFactory.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/DS_HuffmanEncodingTreeNode.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/DS_LinkedList.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/DS_List.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/DS_Map.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/DS_MemoryPool.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/DS_Multilist.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/DS_OrderedChannelHeap.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/DS_OrderedList.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/DS_Queue.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/DS_QueueLinkedList.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/DS_RangeList.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/DS_Table.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/DS_ThreadsafeAllocatingQueue.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/DS_Tree.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/DS_WeightedGraph.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/DynDNS.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/EmailSender.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/EmptyHeader.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/EpochTimeToString.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/Export.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/FileList.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/FileListNodeContext.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/FileListTransfer.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/FileListTransferCBInterface.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/FileOperations.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/FormatString.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/FullyConnectedMesh2.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/Getche.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/Gets.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/GetTime.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/gettimeofday.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/GridSectorizer.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/HTTPConnection.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/HTTPConnection2.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/IncrementalReadInterface.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/InternalPacket.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/Itoa.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/Kbhit.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/LinuxStrings.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/LocklessTypes.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/LogCommandParser.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/MessageFilter.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/MessageIdentifiers.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/MTUSize.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/NativeFeatureIncludes.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/NativeFeatureIncludesOverrides.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/NativeTypes.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/NatPunchthroughClient.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/NatPunchthroughServer.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/NatTypeDetectionClient.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/NatTypeDetectionCommon.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/NatTypeDetectionServer.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/NetworkIDManager.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/NetworkIDObject.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/PacketConsoleLogger.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/PacketFileLogger.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/PacketizedTCP.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/PacketLogger.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/PacketOutputWindowLogger.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/PacketPool.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/PacketPriority.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/PluginInterface2.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/PS3Includes.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/PS4Includes.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/Rackspace.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/RakAlloca.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/RakAssert.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/RakMemoryOverride.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/RakNetCommandParser.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/RakNetDefines.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/RakNetDefinesOverrides.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/RakNetSmartPtr.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/RakNetSocket.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/RakNetSocket2.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/RakNetStatistics.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/RakNetTime.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/RakNetTransport2.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/RakNetTypes.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/RakNetVersion.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/RakPeer.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/RakPeerInterface.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/RakSleep.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/RakString.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/RakThread.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/RakWString.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/Rand.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/RandSync.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/ReadyEvent.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/RefCountedObj.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/RelayPlugin.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/ReliabilityLayer.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/ReplicaEnums.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/ReplicaManager3.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/Router2.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/RPC4Plugin.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/SecureHandshake.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/SendToThread.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/SignaledEvent.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/SimpleMutex.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/SimpleTCPServer.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/SingleProducerConsumer.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/SocketDefines.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/SocketIncludes.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/SocketLayer.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/StatisticsHistory.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/StringCompressor.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/StringTable.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/SuperFastHash.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/TableSerializer.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/TCPInterface.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/TeamBalancer.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/TeamManager.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/TelnetTransport.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/ThreadPool.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/ThreadsafePacketLogger.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/TransportInterface.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/TwoWayAuthentication.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/UDPForwarder.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/UDPProxyClient.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/UDPProxyCommon.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/UDPProxyCoordinator.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/UDPProxyServer.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/VariableDeltaSerializer.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/VariableListDeltaTracker.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/VariadicSQLParser.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/VitaIncludes.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/WindowsIncludes.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/WSAStartupSingleton.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/XBox360Includes.h;/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet/_FindFirst.h")
  IF (CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  ENDIF (CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
  IF (CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  ENDIF (CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
FILE(INSTALL DESTINATION "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/include/raknet" TYPE FILE FILES
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/AutopatcherPatchContext.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/AutopatcherRepositoryInterface.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/Base64Encoder.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/BitStream.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/CCRakNetSlidingWindow.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/CCRakNetUDT.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/CheckSum.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/CloudClient.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/CloudCommon.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/CloudServer.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/CommandParserInterface.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/ConnectionGraph2.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/ConsoleServer.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/DataCompressor.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/DirectoryDeltaTransfer.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/DR_SHA1.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/DS_BinarySearchTree.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/DS_BPlusTree.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/DS_BytePool.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/DS_ByteQueue.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/DS_Hash.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/DS_Heap.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/DS_HuffmanEncodingTree.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/DS_HuffmanEncodingTreeFactory.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/DS_HuffmanEncodingTreeNode.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/DS_LinkedList.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/DS_List.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/DS_Map.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/DS_MemoryPool.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/DS_Multilist.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/DS_OrderedChannelHeap.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/DS_OrderedList.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/DS_Queue.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/DS_QueueLinkedList.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/DS_RangeList.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/DS_Table.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/DS_ThreadsafeAllocatingQueue.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/DS_Tree.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/DS_WeightedGraph.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/DynDNS.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/EmailSender.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/EmptyHeader.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/EpochTimeToString.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/Export.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/FileList.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/FileListNodeContext.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/FileListTransfer.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/FileListTransferCBInterface.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/FileOperations.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/FormatString.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/FullyConnectedMesh2.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/Getche.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/Gets.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/GetTime.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/gettimeofday.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/GridSectorizer.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/HTTPConnection.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/HTTPConnection2.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/IncrementalReadInterface.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/InternalPacket.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/Itoa.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/Kbhit.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/LinuxStrings.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/LocklessTypes.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/LogCommandParser.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/MessageFilter.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/MessageIdentifiers.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/MTUSize.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/NativeFeatureIncludes.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/NativeFeatureIncludesOverrides.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/NativeTypes.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/NatPunchthroughClient.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/NatPunchthroughServer.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/NatTypeDetectionClient.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/NatTypeDetectionCommon.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/NatTypeDetectionServer.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/NetworkIDManager.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/NetworkIDObject.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/PacketConsoleLogger.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/PacketFileLogger.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/PacketizedTCP.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/PacketLogger.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/PacketOutputWindowLogger.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/PacketPool.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/PacketPriority.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/PluginInterface2.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/PS3Includes.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/PS4Includes.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/Rackspace.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/RakAlloca.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/RakAssert.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/RakMemoryOverride.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/RakNetCommandParser.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/RakNetDefines.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/RakNetDefinesOverrides.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/RakNetSmartPtr.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/RakNetSocket.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/RakNetSocket2.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/RakNetStatistics.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/RakNetTime.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/RakNetTransport2.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/RakNetTypes.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/RakNetVersion.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/RakPeer.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/RakPeerInterface.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/RakSleep.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/RakString.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/RakThread.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/RakWString.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/Rand.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/RandSync.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/ReadyEvent.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/RefCountedObj.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/RelayPlugin.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/ReliabilityLayer.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/ReplicaEnums.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/ReplicaManager3.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/Router2.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/RPC4Plugin.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/SecureHandshake.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/SendToThread.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/SignaledEvent.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/SimpleMutex.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/SimpleTCPServer.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/SingleProducerConsumer.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/SocketDefines.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/SocketIncludes.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/SocketLayer.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/StatisticsHistory.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/StringCompressor.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/StringTable.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/SuperFastHash.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/TableSerializer.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/TCPInterface.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/TeamBalancer.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/TeamManager.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/TelnetTransport.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/ThreadPool.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/ThreadsafePacketLogger.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/TransportInterface.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/TwoWayAuthentication.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/UDPForwarder.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/UDPProxyClient.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/UDPProxyCommon.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/UDPProxyCoordinator.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/UDPProxyServer.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/VariableDeltaSerializer.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/VariableListDeltaTracker.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/VariadicSQLParser.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/VitaIncludes.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/WindowsIncludes.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/WSAStartupSingleton.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/XBox360Includes.h"
    "/mnt/c/Repos/GTA-Orange-Reloaded/deps/RakNet/Source/_FindFirst.h"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

