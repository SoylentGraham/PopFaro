#include "PopFaro.h"
#include <TParameters.h>
#include <SoyDebug.h>
#include <TProtocolCli.h>
#include <TProtocolHttp.h>
#include <SoyApp.h>
#include <PopMain.h>
#include <TJobRelay.h>
#include <SoyPixels.h>
#include <SoyString.h>
#include <TFeatureBinRing.h>
#include <SortArray.h>
#include <TChannelFile.h>

#include "TFaroLightScanner.h"


TPopOculus::TPopOculus() :
	TJobHandler		( static_cast<TChannelManager&>(*this) ),
	TPopJobHandler	( static_cast<TJobHandler&>(*this) )
{
	AddJobHandler("exit", TParameterTraits(), *this, &TPopOculus::OnExit );

	TParameterTraits ParseParameterTraits;
	ParseParameterTraits.mAssumedKeys.PushBack("default");
	ParseParameterTraits.mDefaultParams.PushBack( std::make_tuple("default_format","file/binary") );
	AddJobHandler("parsefsl", ParseParameterTraits, *this, &TPopOculus::OnParseFsl );
}

bool TPopOculus::AddChannel(std::shared_ptr<TChannel> Channel)
{
	if ( !TChannelManager::AddChannel( Channel ) )
		return false;
	TJobHandler::BindToChannel( *Channel );
	return true;
}


void TPopOculus::OnExit(TJobAndChannel& JobAndChannel)
{
	mConsoleApp.Exit();
	
	//	should probably still send a reply
	TJobReply Reply( JobAndChannel );
	Reply.mParams.AddDefaultParam(std::string("exiting..."));
	TChannel& Channel = JobAndChannel;
	Channel.OnJobCompleted( Reply );
}



void TPopOculus::OnParseFsl(TJobAndChannel& JobAndChannel)
{
	TJobReply Reply( JobAndChannel );

	std::Debug << JobAndChannel.GetJob().mParams << std::endl;
	
	//	load binary from payload
	Array<char> Binary;
	if ( !JobAndChannel.GetJob().mParams.GetParamAs(TJobParam::Param_Default,Binary) )
	{
		Reply.mParams.AddErrorParam("Failed to get binary data");
		TChannel& Channel = JobAndChannel;
		Channel.OnJobCompleted( Reply );
		return;
	}

	//	parse
	TPointCloud PointCloud;
	std::stringstream Error;
	if ( TFileFaroLightScanner::Parse( PointCloud, GetArrayBridge(Binary), Error ) )
	{
		//	push point cloud data
	}

	if ( !Error.str().empty() )
	{
		Reply.mParams.AddErrorParam( Error.str() );
	}

	TChannel& Channel = JobAndChannel;
	Channel.OnJobCompleted( Reply );
}
	



TPopAppError::Type PopMain(TJobParams& Params)
{
	TPopOculus App;

	//	create stdio channel for commandline output
	auto StdioChannel = CreateChannelFromInputString("std:", SoyRef("stdio") );
	auto HttpChannel = CreateChannelFromInputString("http:8080-8090", SoyRef("http") );
	
	App.AddChannel( StdioChannel );
	App.AddChannel( HttpChannel );
	
	
	//	bootup commands via a channel
	std::shared_ptr<TChannel> BootupChannel( new TChan<TChannelFileRead,TProtocolCli>( SoyRef("Bootup"), "bootup.txt" ) );
	
	//	display reply to stdout
	//	when the commandline SENDs a command (a reply), send it to stdout
	auto RelayFunc = [](TJobAndChannel& JobAndChannel)
	{
		std::Debug << JobAndChannel.GetJob().mParams << std::endl;
	};
	//BootupChannel->mOnJobRecieved.AddListener( RelayFunc );
	BootupChannel->mOnJobSent.AddListener( RelayFunc );
	BootupChannel->mOnJobLost.AddListener( RelayFunc );
	
	App.AddChannel( BootupChannel );

	
	
	
	//	run
	App.mConsoleApp.WaitForExit();

	StdioChannel.reset();
	return TPopAppError::Success;
}




