#include "pch.h"
#include "sdk.h"

#include "../CoreUObject/UObject/UObjectBaseUtility.h"
#include "../CoreUObject/UObject/UnrealTypePrivate.h"
#include <iostream>
#include "formatting.h"

// Returns a string because if you do a char it breaks stuff
static std::string GetPrefix(UObjectBase* Object) // TODO: Move this to SDKFormatting;
{
	static UClass* UClassClass = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.Class");

	if (((UObjectBaseUtility*)Object)->IsA(UClassClass))
	{
		return ((UClass*)Object)->GetPrefixCPP();
	}

	return "F";
}

// Best sorting algorithm lmfao
static std::string UPropertyTypeToStringHacky(UProperty* Property) {
	static UClass* DoubleProp = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.DoubleProperty");
	static UClass* FloatProp = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.FloatProperty");
	static UClass* IntProp = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.IntProperty");
	static UClass* Int16Prop = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.Int16Property");
	static UClass* BoolProp = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.BoolProperty");
	static UClass* ObjectProp = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.ObjectProperty");
	static UClass* FunctionProp = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.Function");
	static UClass* StructProp = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.StructProperty");
	static UClass* ClassProp = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.ClassProperty");
	static UClass* ArrayProp = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.ArrayProperty");
	static UClass* ByteProp = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.ByteProperty");
	static UClass* MulticastDelegateProp = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.MulticastDelegateProperty");
	static UClass* EnumProp = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.EnumProperty");
	static UClass* StrProp = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.StrProperty");
	static UClass* NameProp = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.NameProperty");
	static UClass* UInt32Prop = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.UInt32Property");
	static UClass* UInt64Prop = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.UInt64Property");
	static UClass* Int64Prop = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.Int64Property");
	static UClass* Int8Prop = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.Int8Property");
	static UClass* TextProp = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.TextProperty");
	static UClass* SoftObjectProp = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.SoftObjectProperty");
	static UClass* SoftClassProp = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.SoftClassProperty");
	static UClass* WeakObjectProp = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.WeakObjectProperty");
	static UClass* LazyObjectProp = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.LazyObjectProperty");

	UClass* ClassPrivate = Property->GetClass();

	// "Easy" props first
	if (ClassPrivate == DoubleProp)
		return "double";
	else if (ClassPrivate == FloatProp)
		return "float";
	else if (ClassPrivate == IntProp)
		return "int";
	else if (ClassPrivate == Int16Prop)
		return "int16_t";
	else if (ClassPrivate == UInt32Prop)
		return "uint32_t";
	else if (ClassPrivate == NameProp)
		return "FName";
	else if (ClassPrivate == StrProp)
		return "FString";
	else if (ClassPrivate == UInt64Prop)
		return "uint64_t";
	else if (ClassPrivate == Int64Prop)
		return "int64_t";
	else if (ClassPrivate == Int8Prop)
		return "int8_t";
	else if (ClassPrivate == TextProp)
		return "struct FText";
	else {
		// SOOOO, now we need to make sure the struct is already in the file

		if (ClassPrivate == ArrayProp) {
			UArrayProperty* ArrayProperty = (UArrayProperty*)Property;

			return std::format("TArray<{}>", UPropertyTypeToStringHacky(ArrayProperty->GetInner()));
		}
		else if (ClassPrivate == EnumProp) {
			UEnumProperty* Enum = (UEnumProperty*)Property;

			return Utils::UKismetStringLibrary::Conv_NameToString(Enum->GetEnum()->GetFName()).ToString();
		}
		else if (ClassPrivate == StructProp) {
			UStructProperty* StructProperty = (UStructProperty*)Property;

			return "struct " + GetPrefix(StructProperty) + Utils::UKismetStringLibrary::Conv_NameToString(StructProperty->GetStruct()->GetFName()).ToString() + ((Property->GetArrayDim() > 1) ? "*" : "");
		}
		else if (ClassPrivate == ClassProp) {
			auto MetaClass = *(UStruct**)(__int64(Property) + 120);
			// Classes are always pointers?
			return "class " + GetPrefix(MetaClass) + Utils::UKismetStringLibrary::Conv_NameToString(MetaClass->GetFName()).ToString() + "*"/* + ((Property->GetArrayDim() > 1) ? "*" : "")*/;
		}
		else if (ClassPrivate == BoolProp) {
			// todo: do some bitfield stuff kms
			return "bool";
		}
		else if (ClassPrivate == ByteProp)
			return "char";	// return std::format("TEnumAsByte<{}>", Utils::UKismetStringLibrary::Conv_NameToString(Property->GetFName()).ToString());


		// UObjectPropertyBase
		else if (ClassPrivate == ObjectProp) {
			UObjectPropertyBase* ObjectProperty = (UObjectPropertyBase*)Property;

			auto PropertyClass = ObjectProperty->GetPropertyClass();
			if (PropertyClass) {
				return "class " + GetPrefix(PropertyClass) + Utils::UKismetStringLibrary::Conv_NameToString(PropertyClass->GetFName()).ToString() + ((Property->GetArrayDim()) ? "*" : "");
			}
			else {
				return "MILXNOR?";
			}
		}
		else if (ClassPrivate == SoftObjectProp) {
			USoftObjectProperty* SoftObjectProperty = (USoftObjectProperty*)Property;

			auto PropertyClass = SoftObjectProperty->GetPropertyClass();
			if (PropertyClass) {
				std::string inner = GetPrefix(PropertyClass) + Utils::UKismetStringLibrary::Conv_NameToString(PropertyClass->GetFName()).ToString() + ((Property->GetArrayDim()) ? "*" : "");
#ifdef INCLUDE_IN_UE
				return std::format("struct TSoftObjectPtr<struct {}>", inner);
#else
				return inner;
#endif
			}
			else {
				return "FAILED";
			}
		} /*else if (ClassPrivate == SoftObjectProp) { // TODO: MetaClass
			USoftClassProperty* SoftClassProperty = (USoftClassProperty*)Property;

			auto PropertyClass = SoftClassProperty->GetPropertyClass();
			if (PropertyClass) {
				std::string inner = GetPrefix(PropertyClass) + Utils::UKismetStringLibrary::Conv_NameToString(PropertyClass->GetFName()).ToString() + ((Property->GetArrayDim()) ? "*" : "");
				#ifdef INCLUDE_IN_UE
					return std::format("TSoftClassPtr<{}>", inner);
				#else
					return inner;
				#endif
			}
			else {
				return "FAILED";
			}
		}*/ else if (ClassPrivate == LazyObjectProp) {
			ULazyObjectProperty* LazyObjectProperty = (ULazyObjectProperty*)Property;

			auto PropertyClass = LazyObjectProperty->GetPropertyClass();
			if (PropertyClass) {
				std::string inner = GetPrefix(PropertyClass) + Utils::UKismetStringLibrary::Conv_NameToString(PropertyClass->GetFName()).ToString() + ((Property->GetArrayDim()) ? "*" : "");
#ifdef INCLUDE_IN_UE
				return std::format("TLazyObjectPtr<{}>", inner);
#else
				return inner;
#endif
			}
			else {
				return "FAILED";
			}
		}
		else if (ClassPrivate == WeakObjectProp) {
			UWeakObjectProperty* WeakObjectProperty = (UWeakObjectProperty*)Property;

			auto PropertyClass = WeakObjectProperty->GetPropertyClass();
			if (PropertyClass) {
				std::string inner = GetPrefix(PropertyClass) + Utils::UKismetStringLibrary::Conv_NameToString(PropertyClass->GetFName()).ToString() + ((Property->GetArrayDim()) ? "*" : "");
#ifdef INCLUDE_IN_UE
				return std::format("TWeakObjectPtr<{}>", inner);
#else
				return inner;
#endif
			}
			else {
				return "FAILED";
			}
		}

		// Functions

		if (ClassPrivate == FunctionProp) {
			std::string ReturnValueType;
			UFunction* Function = (UFunction*)Property;

			EFunctionFlags FunctionFlags = Function->GetFunctionFlags();

			std::string ReturnType = "";
			std::vector<std::pair<std::string, std::string>> Params; // Param Type, Param Name

			for (UProperty* Parameter = (UProperty*)Function->GetChildren(); Parameter; Parameter = (UProperty*)Parameter->GetNext())
			{
				auto PropertyFlags = *(uint64_t*)(__int64(Parameter) + 0x38);
				auto ArrayDim = *(uint32_t*)(__int64(Parameter) + 0x30);
				auto ParamType = UPropertyTypeToStringHacky((UObjectPropertyBase*)Parameter);

				if (PropertyFlags & 0x400)
				{
					ReturnType = ParamType;
				}
				else if (PropertyFlags & 0x80) // Param Flag
				{
					Params.push_back({
						ParamType,
						Utils::UKismetStringLibrary::Conv_NameToString(Parameter->GetFName()).ToString()
						});
				}
			}

			if (ReturnType == "")
				ReturnType = "void";

			auto FullFunction = std::format("{} {}(", ReturnType, Utils::UKismetStringLibrary::Conv_NameToString(((UObjectPropertyBase*)Property)->GetFName()).ToString());
			for (int i = 0; i < Params.size(); i++)
			{
				auto& Param = Params[i];
				FullFunction += Param.first + ' ' + Param.second;
				if (i != Params.size() - 1)
					FullFunction += ", ";
			}

			return (FunctionFlags & EFunctionFlags::FUNC_Static) ? "static " + FullFunction : FullFunction;
		}
		else if (ClassPrivate == MulticastDelegateProp) {
			// Get the UFunction
			UMulticastDelegateProperty* Delegate = (UMulticastDelegateProperty*)Property;
			UFunction* Function = Delegate->GetSignatureFunction();

			if (Function) {
				// Note: "Function" won't be in the SDK dump? (Maybe a bug when filtering out?)
				// for now lets just add them manually

				// Probably, bc SuperStuct is null?

			//	printf("Function: %p\n", Utils::UKismetSystemLibrary::GetPathName((uintptr_t*)Function).ToString().c_str());

				// if (Function->GetClass() == Delegate) {

					//printf(Utils::UKismetSystemLibrary::GetPathName((uintptr_t*)Function).ToString().c_str());

					//printf("Function: %p\n", Function);
					//printf("Func: %p\n", Function->GetFunc());
					// TODO: Take a deeper look into it...

				UDelegateProperty* DelegateProperty = (UDelegateProperty*)Function;


				//	printf("DelegateProperty: %p\n", DelegateProperty);


				//	printf("Func: %p\n", DelegateProperty->GetSignatureFunction());
				//	printf("Outer: %s\n", Utils::UKismetSystemLibrary::GetPathName((uintptr_t*)DelegateProperty->GetOuter()).ToString().c_str());
				// }


			}



			return Utils::UKismetStringLibrary::Conv_NameToString(Property->GetClass()->GetFName()).ToString();
		}
		else {
			return "__int64" + std::string("/*") + Utils::UKismetStringLibrary::Conv_NameToString(Property->GetClass()->GetFName()).ToString() + "*/";
		}
	}
}

std::string SDKFormatting::UPropertyTypeToString(UProperty* Property) {
	static UClass* DoubleProp = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.DoubleProperty");
	static UClass* FloatProp = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.FloatProperty");
	static UClass* IntProp = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.IntProperty");
	static UClass* Int16Prop = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.Int16Property");
	static UClass* BoolProp = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.BoolProperty");
	static UClass* ObjectProp = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.ObjectProperty");
	static UClass* FunctionProp = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.Function");
	static UClass* StructProp = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.StructProperty");
	static UClass* ClassProp = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.ClassProperty");
	static UClass* ArrayProp = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.ArrayProperty");
	static UClass* ByteProp = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.ByteProperty");
	static UClass* MulticastDelegateProp = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.MulticastDelegateProperty");
	static UClass* EnumProp = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.EnumProperty");
	static UClass* StrProp = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.StrProperty");
	static UClass* NameProp = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.NameProperty");
	static UClass* UInt32Prop = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.UInt32Property");
	static UClass* UInt64Prop = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.UInt64Property");
	static UClass* Int64Prop = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.Int64Property");
	static UClass* Int8Prop = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.Int8Property");
	static UClass* TextProp = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.TextProperty");
	static UClass* SoftObjectProp = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.SoftObjectProperty");
	static UClass* SoftClassProp = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.SoftClassProperty");
	static UClass* WeakObjectProp = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.WeakObjectProperty");
	static UClass* LazyObjectProp = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.LazyObjectProperty");

	UClass* ClassPrivate = Property->GetClass();

	// "Easy" props first
	if (ClassPrivate == DoubleProp)
		return "double";
	else if (ClassPrivate == FloatProp)
		return "float";
	else if (ClassPrivate == IntProp)
		return "int";
	else if (ClassPrivate == Int16Prop)
		return "int16_t";
	else if (ClassPrivate == UInt32Prop)
		return "uint32_t";
	else if (ClassPrivate == NameProp)
		return "FName";
	else if (ClassPrivate == StrProp)
		return "FString";
	else if (ClassPrivate == UInt64Prop)
		return "uint64_t";
	else if (ClassPrivate == Int64Prop)
		return "int64_t";
	else if (ClassPrivate == Int8Prop)
		return "int8_t";
	else if (ClassPrivate == TextProp)
		return "struct FText";

	// TODO: Clean up a bit, maybe speed up too (yea, speedup...)

	else if (ClassPrivate == ArrayProp) {
		UArrayProperty* ArrayProperty = (UArrayProperty*)Property;

		return std::format("TArray<{}>", UPropertyTypeToString(ArrayProperty->GetInner()));
	} else if (ClassPrivate == EnumProp) {
		UEnumProperty* Enum = (UEnumProperty*)Property;

		return Utils::UKismetStringLibrary::Conv_NameToString(Enum->GetEnum()->GetFName()).ToString();
	} else if (ClassPrivate == StructProp) {
		UStructProperty* StructProperty = (UStructProperty*)Property;
		
		return "struct " + GetPrefix(StructProperty) + Utils::UKismetStringLibrary::Conv_NameToString(StructProperty->GetStruct()->GetFName()).ToString() + ((Property->GetArrayDim() > 1) ? "*" : "");
	} else if (ClassPrivate == ClassProp) {
		auto MetaClass = *(UStruct**)(__int64(Property) + 120);
		// Classes are always pointers?
		return "class " + GetPrefix(MetaClass) + Utils::UKismetStringLibrary::Conv_NameToString(MetaClass->GetFName()).ToString() + "*"/* + ((Property->GetArrayDim() > 1) ? "*" : "")*/;
	}
	else if (ClassPrivate == BoolProp) {
		// todo: do some bitfield stuff kms
		return "bool";
	} else if (ClassPrivate == ByteProp)
		return "char";	// return std::format("TEnumAsByte<{}>", Utils::UKismetStringLibrary::Conv_NameToString(Property->GetFName()).ToString());

	
	// UObjectPropertyBase
	else if (ClassPrivate == ObjectProp) {
		UObjectPropertyBase* ObjectProperty = (UObjectPropertyBase*)Property;

		auto PropertyClass = ObjectProperty->GetPropertyClass();
		if (PropertyClass) {
			return "class " + GetPrefix(PropertyClass) + Utils::UKismetStringLibrary::Conv_NameToString(PropertyClass->GetFName()).ToString() + ((Property->GetArrayDim()) ? "*" : "");
		}
		else {
			return "MILXNOR?";
		}
	} else if (ClassPrivate == SoftObjectProp) {
		USoftObjectProperty* SoftObjectProperty = (USoftObjectProperty*)Property;

		auto PropertyClass = SoftObjectProperty->GetPropertyClass();
		if (PropertyClass) {
			std::string inner = GetPrefix(PropertyClass) + Utils::UKismetStringLibrary::Conv_NameToString(PropertyClass->GetFName()).ToString() + ((Property->GetArrayDim()) ? "*" : "");
			#ifdef INCLUDE_IN_UE
				return std::format("struct TSoftObjectPtr<struct {}>", inner);
			#else
				return inner;
			#endif
		}
		else {
			return "FAILED";
		}
	} /*else if (ClassPrivate == SoftObjectProp) { // TODO: MetaClass
		USoftClassProperty* SoftClassProperty = (USoftClassProperty*)Property;

		auto PropertyClass = SoftClassProperty->GetPropertyClass();
		if (PropertyClass) {
			std::string inner = GetPrefix(PropertyClass) + Utils::UKismetStringLibrary::Conv_NameToString(PropertyClass->GetFName()).ToString() + ((Property->GetArrayDim()) ? "*" : "");
			#ifdef INCLUDE_IN_UE
				return std::format("TSoftClassPtr<{}>", inner);
			#else
				return inner;
			#endif
		}
		else {
			return "FAILED";
		}
	}*/ else if (ClassPrivate == LazyObjectProp) {
		ULazyObjectProperty* LazyObjectProperty = (ULazyObjectProperty*)Property;

		auto PropertyClass = LazyObjectProperty->GetPropertyClass();
		if (PropertyClass) {
			std::string inner = GetPrefix(PropertyClass) + Utils::UKismetStringLibrary::Conv_NameToString(PropertyClass->GetFName()).ToString() + ((Property->GetArrayDim()) ? "*" : "");
			#ifdef INCLUDE_IN_UE
				return std::format("TLazyObjectPtr<{}>", inner);
			#else
				return inner;
			#endif
		}
		else {
			return "FAILED";
		}
	} else if (ClassPrivate == WeakObjectProp) {
		UWeakObjectProperty* WeakObjectProperty = (UWeakObjectProperty*)Property;

		auto PropertyClass = WeakObjectProperty->GetPropertyClass();
		if (PropertyClass) {
			std::string inner = GetPrefix(PropertyClass) + Utils::UKismetStringLibrary::Conv_NameToString(PropertyClass->GetFName()).ToString() + ((Property->GetArrayDim()) ? "*" : "");
			#ifdef INCLUDE_IN_UE
				return std::format("TWeakObjectPtr<{}>", inner);
			#else
				return inner;
			#endif
		}
		else {
			return "FAILED";
		}
	}
	
	// Functions
	
	if (ClassPrivate == FunctionProp) {
		std::string ReturnValueType;
		UFunction* Function = (UFunction*)Property;

		EFunctionFlags FunctionFlags = Function->GetFunctionFlags();

		std::string ReturnType = "";
		std::vector<std::pair<std::string, std::string>> Params; // Param Type, Param Name

		for (UProperty* Parameter = (UProperty*)Function->GetChildren(); Parameter; Parameter = (UProperty*)Parameter->GetNext())
		{
			auto PropertyFlags = *(uint64_t*)(__int64(Parameter) + 0x38);
			auto ArrayDim = *(uint32_t*)(__int64(Parameter) + 0x30);
			auto ParamType = UPropertyTypeToString((UObjectPropertyBase*)Parameter);

			if (PropertyFlags & 0x400)
			{
				ReturnType = ParamType;
			}
			else if (PropertyFlags & 0x80) // Param Flag
			{
				Params.push_back({
					ParamType,
					Utils::UKismetStringLibrary::Conv_NameToString(Parameter->GetFName()).ToString()
				});
			}
		}

		if (ReturnType == "")
			ReturnType = "void";

		auto FullFunction = std::format("{} {}(", ReturnType, Utils::UKismetStringLibrary::Conv_NameToString(((UObjectPropertyBase*)Property)->GetFName()).ToString());
		for (int i = 0; i < Params.size(); i++)
		{
			auto& Param = Params[i];
			FullFunction += Param.first + ' ' + Param.second;
			if (i != Params.size() - 1)
				FullFunction += ", ";
		}

		return (FunctionFlags & EFunctionFlags::FUNC_Static) ? "static " + FullFunction : FullFunction;
	}
	 else if (ClassPrivate == MulticastDelegateProp) {
		// Get the UFunction
		UMulticastDelegateProperty* Delegate = (UMulticastDelegateProperty*)Property;
		UFunction* Function = Delegate->GetSignatureFunction();

		if (Function) { 
			// Note: "Function" won't be in the SDK dump? (Maybe a bug when filtering out?)
			// for now lets just add them manually

			// Probably, bc SuperStuct is null?
			
		//	printf("Function: %p\n", Utils::UKismetSystemLibrary::GetPathName((uintptr_t*)Function).ToString().c_str());
			
			// if (Function->GetClass() == Delegate) {
				
				//printf(Utils::UKismetSystemLibrary::GetPathName((uintptr_t*)Function).ToString().c_str());

				//printf("Function: %p\n", Function);
				//printf("Func: %p\n", Function->GetFunc());
				// TODO: Take a deeper look into it...

				UDelegateProperty* DelegateProperty = (UDelegateProperty*)Function;


			//	printf("DelegateProperty: %p\n", DelegateProperty);
				

			//	printf("Func: %p\n", DelegateProperty->GetSignatureFunction());
			//	printf("Outer: %s\n", Utils::UKismetSystemLibrary::GetPathName((uintptr_t*)DelegateProperty->GetOuter()).ToString().c_str());
			// }
		}
		
		return "struct FScriptMulticastDelegate"; // Stolen from kn4cker, i think this can be done better but too lazy rn
		return Utils::UKismetStringLibrary::Conv_NameToString(Property->GetClass()->GetFName()).ToString();
	}
	 else {
		return "__int64" + std::string("/*") + Utils::UKismetStringLibrary::Conv_NameToString(Property->GetClass()->GetFName()).ToString() + "*/";
	}
}

std::string GetFullCPPName() // TODO: Do this
{
	return "";
}

void SDKFormatting::LupusFormatUClass(UClass* Class, Ofstreams* streams) {
	static UClass* FunctionClass = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.Function");
	static UClass* BoolProp = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.BoolProperty");

	// Get the proper name
	UStruct* SuperStruct = Class->GetSuperStruct();

	std::string name = Utils::UKismetSystemLibrary::GetPathName((uintptr_t*)Class).ToString();
	#ifdef INCLUDE_IN_UE
		streams->Classes << "\n\nUCLASS(BlueprintType)\n";
	#endif
	if (SuperStruct) {
		streams->Classes << std::format(
			"class {}{}", GetPrefix(Class) + ((UObjectBaseUtility*)Class)->GetName().ToString(), // TODO: Use split and then get the stuff
			" : public " + GetPrefix(Class->GetSuperStruct()) + ((UObjectBaseUtility*)Class->GetSuperStruct())->GetName().ToString()
		);
	} else {
		streams->Classes << std::format(
			"class {}", GetPrefix(Class) + ((UObjectBaseUtility*)Class)->GetName().ToString()
		);
	}

	streams->Classes << "\n{\n	public:\n";

	int offset = 0;
	int unknownProps = 0;
	if (Class->GetChildren()) {
		offset = ((UProperty*)Class->GetChildren())->GetOffset_Internal();

		if (SuperStruct) {
			if (SuperStruct->GetPropertiesSize() < offset) {
				streams->Classes << std::format("	    char UnknownData{}[0x{:x}];\n", unknownProps, offset - SuperStruct->GetPropertiesSize());
				unknownProps++;
			}
		}

		for (UField* Property = (UField*)Class->GetChildren(); Property; Property = Property ? Property->GetNext() : 0) {
			std::string propName = Utils::UKismetStringLibrary::Conv_NameToString(((UObjectPropertyBase*)Property)->GetFName()).ToString();

			if (Property->GetClass() == FunctionClass) {
				// TODO: FUNCTION!!
				// streams->Classes << std::format("	    {} {}; // 0x{:x} Size: 0x{:x}\n", "void", propName, ((UProperty*)Property)->GetOffset_Internal(), ((UProperty*)Property)->GetElementSize());

				offset += ((UProperty*)Property)->GetElementSize();
			} else {
				if (
					Property->GetClass() != BoolProp &&
					((UProperty*)Property)->GetOffset_Internal() != offset
				) {
					if (((UProperty*)Property)->GetOffset_Internal() < offset) {
						// Logic issue?
						streams->Classes << std::format(
							"	    char UnknownData{}[0x{:x}]; // 0x{:x}\n", unknownProps,
							-(((UProperty*)Property)->GetOffset_Internal() - offset),
							offset
						);
					} else {
						streams->Classes << std::format(
							"	    char UnknownData{}[0x{:x}]; // 0x{:x}\n", unknownProps,
							((UProperty*)Property)->GetOffset_Internal() - offset,
							offset
						);
					}

					offset = ((UProperty*)Property)->GetOffset_Internal();
					unknownProps++;
				}

				std::string propType = UPropertyTypeToString((UObjectPropertyBase*)Property);
				streams->Classes << std::format("	    {} {}; // 0x{:x} Size: 0x{:x}\n", propType, propName, ((UProperty*)Property)->GetOffset_Internal(), ((UProperty*)Property)->GetElementSize());

				offset += ((UProperty*)Property)->GetElementSize();
			}
		}
	}

	if (offset != Class->GetPropertiesSize()) {
		streams->Classes << std::format("	    char UnknownData{}[0x{:x}];\n", unknownProps, Class->GetPropertiesSize() - offset);
		unknownProps++;
	}

	if (Class->GetChildProperties()) {

	}

	#ifndef INCLUDE_IN_UE
		// streams->Classes << "\n		static UClass* StaticClass()\n	    {\n			static auto ptr = UObject::FindClass(\"" << Utils::UKismetSystemLibrary::GetPathName((uintptr_t*)Class).ToString() << "\");\n			return ptr;\n		};\n";
	streams->Classes << "\n		static class UClass* StaticClass()\n	    {\n			return 0;\n		};\n";
	#endif

	streams->Classes << "\n};\n\n";
}

void SDKFormatting::LupusFormatStruct(UClass* Class, Ofstreams* streams, std::unordered_map<__int64, __int64>* packageStructs, bool isEnum, bool isScriptStruct, bool isStruct, std::vector<__int64>* structsAdded, std::unordered_map<__int64, Ofstreams*> allstreams) {
	static UClass* FunctionClass = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.Function");
	static UClass* BoolProp = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.BoolProperty");

	// We are doing this because we can not yet know,
	// if this struct can be used or needs another struct first
	std::string result = "";

	// NOTE: GetName by Milxnor, but it looks wrong to me
	if (isEnum) {
		auto Enum = (UEnum*)Class;
		auto Names = Enum->GetNames();

		if (Names.Num())
		{
			result += std::format("\n\nenum class {} : uint8_t", ((UObjectBaseUtility*)Enum)->GetName().ToString()) + "\n{\n";

			for (int i = 0; i < Names.Num(); i++)
			{
				// result += std::format("\n	{} = {}", Utils::UKismetStringLibrary::Conv_NameToString(Enum->GetNameByIndex(i)).ToString(), i);

				auto& Pair = Names[i];
				auto Name = Pair.Key;
				auto Value = Pair.Value;

				auto NameStr = Utils::UKismetStringLibrary::Conv_NameToString(Name).ToString();

				auto pos = NameStr.find_last_of(':');
				if (pos != std::string::npos) // theres probably a more modern c++ way of doing this
				{
					NameStr = NameStr.substr(pos + 1);
				}

				result += std::format("    {} = {}", NameStr, Value);

				if (i != Names.Num() - 1)
					result += ",\n";
			}

			result += "\n};";
		}
	} else {
		// Get the proper name
		UStruct* SuperStruct = Class->GetSuperStruct();

		std::string name = Utils::UKismetSystemLibrary::GetPathName((uintptr_t*)Class).ToString();
		if (SuperStruct) {
			result += std::format(
				"struct {}{}", GetPrefix(Class) + ((UObjectBaseUtility*)Class)->GetName().ToString(), // TODO: Use split and then get the stuff
				" : public " + GetPrefix(Class->GetSuperStruct()) + ((UObjectBaseUtility*)Class->GetSuperStruct())->GetName().ToString()
			);
		}
		else {
			result += std::format(
				"struct {}", GetPrefix(Class) + ((UObjectBaseUtility*)Class)->GetName().ToString()
			);
		}

		result += "\n{\n	public:\n";

		int offset = 0;
		int unknownProps = 0;
		if (Class->GetChildren()) {
			offset = ((UProperty*)Class->GetChildren())->GetOffset_Internal();

			if (SuperStruct) {
				if (SuperStruct->GetPropertiesSize() < offset) {
					result += std::format("	    char UnknownData{}[0x{:x}];\n", unknownProps, offset - SuperStruct->GetPropertiesSize());
					unknownProps++;
				}
			}

			for (UField* Property = (UField*)Class->GetChildren(); Property; Property = Property ? Property->GetNext() : 0) {
				std::string propName = Utils::UKismetStringLibrary::Conv_NameToString(((UObjectPropertyBase*)Property)->GetFName()).ToString();

				if (Property->GetClass() == FunctionClass) {
					// TODO: FUNCTION!!
					// streams->Classes << std::format("	    {} {}; // 0x{:x} Size: 0x{:x}\n", "void", propName, ((UProperty*)Property)->GetOffset_Internal(), ((UProperty*)Property)->GetElementSize());

					offset += ((UProperty*)Property)->GetElementSize();
				}
				else {
					if (
						Property->GetClass() != BoolProp &&
						((UProperty*)Property)->GetOffset_Internal() != offset
						) {
						if (((UProperty*)Property)->GetOffset_Internal() < offset) {
							// Logic issue?
							result += std::format(
								"	    char UnknownData{}[0x{:x}]; // 0x{:x}\n", unknownProps,
								-(((UProperty*)Property)->GetOffset_Internal() - offset),
								offset
							);
						}
						else {
							result += std::format(
								"	    char UnknownData{}[0x{:x}]; // 0x{:x}\n", unknownProps,
								((UProperty*)Property)->GetOffset_Internal() - offset,
								offset
							);
						}

						offset = ((UProperty*)Property)->GetOffset_Internal();
						unknownProps++;
					}

					static UClass* StructProp = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.StructProperty");
					static UClass* EnumProp = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.EnumProperty");
					if (Property->GetClass() == StructProp) {
						static uint64_t FVectorName = Utils::UKismetStringLibrary::Conv_StringToName(L"Vector").GetComparisonIndex().Value;
						
						// if not already added to the file, we will need to wait
						UStructProperty* StructProp = (UStructProperty*)Property;
						__int64 name = (__int64)StructProp->GetStruct()->GetFName();
						if (name != FVectorName) {
							if (!(std::find(structsAdded->begin(), structsAdded->end(), (__int64)StructProp->GetStruct()->GetFName()) != structsAdded->end())) {
								//printf("Missing: %s\n", Utils::UKismetStringLibrary::Conv_NameToString(StructProp->GetStruct()->GetFName()).ToString().c_str());
								packageStructs->emplace((__int64)StructProp->GetStruct()->GetFName(), (__int64)Class);
								return;
							}
						}
						

					} else if (Property->GetClass() == EnumProp) {
						UEnumProperty* EnumProp = (UEnumProperty*)Property;

						if (!(std::find(structsAdded->begin(), structsAdded->end(), (__int64)EnumProp->GetEnum()->GetFName()) != structsAdded->end())) {
							packageStructs->emplace((__int64)EnumProp->GetEnum()->GetFName(), (__int64)Class);
							return;
						}
					}

					std::string propType = UPropertyTypeToStringHacky((UObjectPropertyBase*)Property);

					result += std::format("	    {} {}; // 0x{:x} Size: 0x{:x}\n", propType, propName, ((UProperty*)Property)->GetOffset_Internal(), ((UProperty*)Property)->GetElementSize());

					offset += ((UProperty*)Property)->GetElementSize();
				}
			}
		}

		if (offset != Class->GetPropertiesSize()) {
			result += std::format("	    char UnknownData{}[0x{:x}];\n", unknownProps, Class->GetPropertiesSize() - offset);
			unknownProps++;
		}

		if (Class->GetChildProperties()) {

		}

		result += "\n};\n\n";
	}

	

	streams->Structs << result;

	structsAdded->emplace_back((__int64)Class->GetFName());
//	printf("Added: %s\n", Utils::UKismetStringLibrary::Conv_NameToString(Class->GetFName()).ToString().c_str());

	// Sorting
	// If this is a package that was needed for another one, 
	// we need to add the other one after this
	for (auto const& [key, val] : *packageStructs)
	{
		//printf(Utils::UKismetSystemLibrary::GetPathName((uintptr_t*)key).ToString().c_str());
		if (Class->GetFName() == key) {
			// if (((UObjectBaseUtility*)val)->IsA(Core))

			Ofstreams* Nestreams = 0;

			for (auto const& [key2, val2] : allstreams)
			{
				if (key2 == (__int64)((UClass*)val)->GetOuter()) {
					Nestreams = val2;
					break;
				}
			}

			if (!Nestreams) {
				printf("FAILED TO FIND THE REAL THINGY\n");
				break;
			}
			
			
			printf("Adding %s because of %s\n", Utils::UKismetStringLibrary::Conv_NameToString(((UClass*)val)->GetFName()).ToString().c_str(), Utils::UKismetStringLibrary::Conv_NameToString(Class->GetFName()).ToString().c_str());

			LupusFormatStruct((UClass*)val, Nestreams, packageStructs, false, true, false, structsAdded, allstreams);
			break;
		}
	}
}

void SDKFormatting::FormatUClass(UClass* Class, Ofstreams* streams) {
	static UClass* FunctionProp = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.Function");
	static UClass* SoftObjectProp = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.SoftObjectProperty");
	static UClass* SoftClassProp = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.SoftClassProperty");
	static UClass* DelegateFuncProp = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.DelegateFunction");
	static UClass* WeakObjProp = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.WeakObjectProperty");
	static UClass* MulticastDelegateProp = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.MulticastDelegateProperty");
	static UClass* SetProp = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.SetProperty");
	static UClass* DelegateProp = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.DelegateProperty");

	std::string result = "";
	std::string funcResult = "";

	std::string Additional = "";
	
	UStruct* SuperStruct = Class->GetSuperStruct();

	if (SuperStruct)
		Additional = " : public " + GetPrefix(Class->GetSuperStruct()) + ((UObjectBaseUtility*)Class->GetSuperStruct())->GetName().ToString();
	else
		Additional = " : public UObject";

	auto name = std::format("class {}{}", GetPrefix(Class) + ((UObjectBaseUtility*)Class)->GetName().ToString(), Additional);

	#ifdef INCLUDE_IN_UE
	result += "\n\nUCLASS(BlueprintType)\n";
	#endif

	result += name + "\n{\npublic:\n";

	if ((Class->GetChildren())) {
		int SuperStructSize = 0;

		if (SuperStruct)
			SuperStructSize = *(uint32_t*)((__int64)SuperStruct + 0x40);

		std::pair<int, int> OldProp; // Offset_Internal, Size
		int padNum = 1;
		

		bool bIsFirstAFunction = (Class->GetChildren()->GetClass() == FunctionProp);
		int OffsetDifference = 0;

		if (!bIsFirstAFunction && SuperStructSize) // make sure it inherits something and it has a member
		{
			OffsetDifference = ((UProperty*)Class->GetChildren())->GetOffset_Internal() - SuperStructSize; // Get the first child's offset and subtract the two

			if (OffsetDifference > 0)
			{
				result += std::format("    char UnknownData{}[0x{:x}];\n", padNum, OffsetDifference);
				padNum++;
			}
		}

		for (UField* Property = (UField*)Class->GetChildren(); Property; Property = Property->GetNext()) {
			std::string pType = UPropertyTypeToString((UObjectPropertyBase*)Property);
			std::string pName = Utils::UKismetStringLibrary::Conv_NameToString(((UObjectPropertyBase*)Property)->GetFName()).ToString();
			bool bIsAFunction = (Property->GetClass() == FunctionProp);

			if (!bIsAFunction)
			{
				auto thisElementSize = *(int32_t*)(__int64(Property) + 0x34);
				bool bUnhandledType = false;

				if ( // Properties we do not have implemented
					Property->GetClass() == SoftClassProp ||
					Property->GetClass() == WeakObjProp ||
					// Property->GetClass() == DelegateFuncProp ||
					Property->GetClass() == DelegateProp ||
					Property->GetClass() == MulticastDelegateProp)
				{
					result += std::format("    char UnknownData{}[0x{:x}]; // UNHANDLED {}\n", padNum, thisElementSize, pType + " " + pName);
					padNum++;
					bUnhandledType = true;
				}

				if (OldProp.first && OldProp.second)
				{
					OffsetDifference = ((UProperty*)Property)->GetOffset_Internal() - (OldProp.first + OldProp.second);
					if (OffsetDifference > 0)
					{
						result += std::format("    char UnknownData{}[0x{:x}];\n", padNum, OffsetDifference);
						padNum++;
					}
				}

				OldProp.first = ((UProperty*)Property)->GetOffset_Internal();
				OldProp.second = thisElementSize;

				auto ArrayDim = ((UProperty*)Property)->GetArrayDim();
				
				if (!bUnhandledType)
				{
					#ifdef INCLUDE_IN_UE
						result += "    " + Formatting::GetUPropertySpecifiers(((UProperty*)Property)->GetPropertyFlags()) + "\n";
					#endif
					pName += (ArrayDim > 1) ? std::format("[0x{:x}]", ArrayDim) : ""; // do like CharacterParts[0x8];

					result += std::format("    {} {}; // 0x{:x}\n", pType, pName, ((UProperty*)Property)->GetOffset_Internal());
				}

				if (Property->GetNext())
					result += "\n";
			}
			else
			{
				UFunction* Function = (UFunction*)Property;

				std::string ReturnValueType;

				EFunctionFlags FunctionFlags = Function->GetFunctionFlags();

				std::string ReturnType = "";
				std::vector<std::pair<std::string, std::string>> Params; // Param Type, Param Name

				for (UProperty* Parameter = (UProperty*)Function->GetChildren(); Parameter; Parameter = (UProperty*)Parameter->GetNext())
				{
					auto PropertyFlags = Parameter->GetPropertyFlags();
					auto ArrayDim = Parameter->GetArrayDim();
					auto ParamType = UPropertyTypeToString((UObjectPropertyBase*)Parameter);

					if (PropertyFlags & 0x400)
					{
						ReturnType = ParamType;
					}
					else if (PropertyFlags & 0x80) // Param Flag
					{
						Params.push_back({
							ParamType,
							Utils::UKismetStringLibrary::Conv_NameToString(Parameter->GetFName()).ToString()
							});
					}
				}

				if (ReturnType == "")
					ReturnType = "void";

				auto FullFunction = std::format("{}(", Utils::UKismetStringLibrary::Conv_NameToString(((UObjectPropertyBase*)Property)->GetFName()).ToString());
				std::string ParamsCombined = "";

				for (int i = 0; i < Params.size(); i++)
				{
					auto& Param = Params[i];
					ParamsCombined += Param.first + ' ' + Param.second;
					if (i != Params.size() - 1)
						ParamsCombined += ", ";
				}

				FullFunction += ParamsCombined;

				auto oldRet = ReturnType; // ReturnType without static
				ReturnType = ((FunctionFlags & EFunctionFlags::FUNC_Static) ? "static " : "") + ReturnType;

				auto nameDef = ReturnType + ' ' + GetPrefix(Class) + ((UObjectBaseUtility*)Class)->GetName().ToString() + "::" + FullFunction; // 0x2000 = STATIC

				funcResult += nameDef + R"()
{)";
				if (Params.size() > 0 || oldRet != "void")
				{
					funcResult += R"(
	struct {
)";
					if (Params.size() > 0)
					{
						for (int i = 0; i < Params.size(); i++)
						{
							auto& Param = Params[i];
							funcResult += "            " + Param.first + ' ' + Param.second + ";";
							if (i != (((Params.size() + ((oldRet == "void") ? 0 : 1))) - 1))
								funcResult += "\n";
						}
					}

					if (ReturnType != "void")
						funcResult += "            " + oldRet + ' ' + "ReturnValue;";

					funcResult += R"(
	} params{ )";
					if (Params.size() > 0)
					{
						for (int i = 0; i < Params.size(); i++)
						{
							auto& Param = Params[i];
							funcResult += Param.second;
							if (i < Params.size() - 1)
								funcResult += ",";
							funcResult += " ";
						}
					}

					funcResult += "};\n";
				}

				// TODO: VTable
				funcResult += "\n    static auto fn = UObject::FindObject<UFunction>(this::StaticClass(), \"" + pName + "\");\n";
				funcResult += "    ProcessEvent(this, fn, " + std::string(((Params.size() > 0) ? "&params" : "nullptr")) + ");";

				if (ReturnType != "void")
					funcResult += "\n\n    return params.ReturnValue; ";

				// END

				funcResult += R"(
}
)" + std::string("\n");





			result += "    " + pType + ");";

			if (Property->GetNext())
				result += "\n";
			}
		}
	}

	#ifndef INCLUDE_IN_UE

		/*result += "\n" + std::string(R"(
		static UClass* StaticClass()
		{
			static UClass* ptr = UObject::FindObject<UClass>(")" + Utils::UKismetSystemLibrary::GetPathName((uintptr_t*)Class).ToString() + R"(");
			return ptr;
		}
	)"); */// i don't know why but format doesnt work
	result += "\n" + std::string(R"(
		static UClass* StaticClass()
		{
			return 0;
		}
	)");
	#endif
		
	result += "\n};";

	streams->Classes << result;
	streams->Functions << funcResult;
}

std::string SDKFormatting::CreateStruct(UStruct* Struct) {
	static UClass* CoreUObjectStruct = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.Struct");
	static UClass* CoreUObjectScriptStruct = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.ScriptStruct");
	static UClass* CoreUObjectEnum = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.Enum");
	static UClass* SoftObjectProp = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.SoftObjectProperty");
	static UClass* SoftClassProp = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.SoftClassProperty");
	static UClass* DelegateFuncProp = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.DelegateFunction");
	static UClass* WeakObjProp = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.WeakObjectProperty");
	static UClass* MulticastDelegateProp = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.MulticastDelegateProperty");
	static UClass* DelegateProp = (UClass*)Utils::StaticFindObject(L"/Script/CoreUObject.DelegateProperty");
	
	std::string result;

	// we are checking what it is twice, which is slow.

	if (((UObjectBaseUtility*)Struct)->IsA(CoreUObjectEnum))
	{
		auto Enum = (UEnum*)Struct;
		auto Names = Enum->GetNames();
		
		if (Names.Num())
		{
			result += std::format("\n\nenum class {} : uint8_t", ((UObjectBaseUtility*)Enum)->GetName().ToString()) + "\n{\n";

			for (int i = 0; i < Names.Num(); i++)
			{
				// result += std::format("\n	{} = {}", Utils::UKismetStringLibrary::Conv_NameToString(Enum->GetNameByIndex(i)).ToString(), i);

				auto& Pair = Names[i];
				auto Name = Pair.Key;
				auto Value = Pair.Value;

				auto NameStr = Utils::UKismetStringLibrary::Conv_NameToString(Name).ToString();

				auto pos = NameStr.find_last_of(':');
				if (pos != std::string::npos) // theres probably a more modern c++ way of doing this
				{
					NameStr = NameStr.substr(pos + 1);
				}

				result += std::format("    {} = {}", NameStr, Value);
				
				if (i != Names.Num() - 1)
					result += ",\n";
			}

			result += "\n};";
		}
	}

	else if (/* ((UObjectBaseUtility*)Struct)->IsA(CoreUObjectStruct) || */ ((UObjectBaseUtility*)Struct)->IsA(CoreUObjectScriptStruct))
	{		
		std::string Additional = "";

		if (Struct->GetSuperStruct())
			Additional = " : public " + GetPrefix(Struct->GetSuperStruct()) + ((UObjectBaseUtility*)Struct->GetSuperStruct())->GetName().ToString();

		auto name = std::format("\n\nstruct {}{}", GetPrefix(Struct) + ((UObjectBaseUtility*)Struct)->GetName().ToString(), Additional);

		result += name + "\n{\n";

		if ((Struct->GetChildren())) { // TODO: basically copy this whole thing from CreateClass when we are done with CreateClass.
			int SuperStructSize = 0;

			if (Struct->GetSuperStruct())
				SuperStructSize = *(uint32_t*)((__int64)Struct->GetSuperStruct() + 0x40);

			std::pair<int, int> OldProp; // Offset_Internal, Size
			int padNum = 1;


			int OffsetDifference = 0;

			if (SuperStructSize) // make sure it inherits something and it has a member
			{
				OffsetDifference = ((UProperty*)Struct->GetChildren())->GetOffset_Internal() - SuperStructSize; // Get the first child's offset and subtract the two

				if (OffsetDifference > 0)
				{
					result += std::format("    char UnknownData{}[0x{:x}];\n", padNum, OffsetDifference);
					padNum++;
				}
			}

			for (UField* Property = (UField*)Struct->GetChildren(); Property; Property = Property->GetNext()) {
				std::string pType = UPropertyTypeToString((UObjectPropertyBase*)Property);
				std::string pName = Utils::UKismetStringLibrary::Conv_NameToString(((UObjectPropertyBase*)Property)->GetFName()).ToString();
				{
					auto thisElementSize = *(int32_t*)(__int64(Property) + 0x34);
					bool bUnhandledType = false;

					if ( // Properties we do not have implemented
						Property->GetClass() == SoftClassProp ||
						Property->GetClass() == WeakObjProp ||
						// Property->GetClass() == DelegateFuncProp ||
						Property->GetClass() == DelegateProp ||
						Property->GetClass() == MulticastDelegateProp)
					{
						result += std::format("    char UnknownData{}[0x{:x}]; // UNHANDLED {}\n", padNum, thisElementSize, pType + " " + pName);
						padNum++;
						bUnhandledType = true;
					}

					if (OldProp.first && OldProp.second)
					{
						OffsetDifference = ((UProperty*)Property)->GetOffset_Internal() - (OldProp.first + OldProp.second);
						if (OffsetDifference > 0)
						{
							result += std::format("    char UnknownData{}[0x{:x}];\n", padNum, OffsetDifference);
							padNum++;
						}
					}

					OldProp.first = ((UProperty*)Property)->GetOffset_Internal();
					OldProp.second = thisElementSize;

					auto ArrayDim = ((UProperty*)Property)->GetArrayDim();

					if (!bUnhandledType)
					{
						#ifdef INLUCDE_IN_UE
							result += "    " + Formatting::GetUPropertySpecifiers(((UProperty*)Property)->GetPropertyFlags()) + "\n";
						#endif
						pName += (ArrayDim > 1) ? std::format("[0x{:x}]", ArrayDim) : ""; // do like CharacterParts[0x8];

						result += std::format("    {} {}; // 0x{:x}\n", pType, pName, ((UProperty*)Property)->GetOffset_Internal());
					}

					if (Property->GetNext())
						result += "\n";
				}
			}
		}
		result += "\n};";
	}

	return result;
}

std::string SDKFormatting::GenerateInitFunction() {
	std::string result = "\n	static void Init() {\n		_StaticFindObject = decltype(_StaticFindObject)((uintptr_t)GetModuleHandle(0) + ";

	auto Base = (uintptr_t)GetModuleHandleW(0);
	std::string staticfindobject = std::format("0x{:x}", (Offsets::StaticFindObject - Base));
	std::string processevent = std::format("0x{:x}", (Offsets::ProcessEvent - Base));

	result += staticfindobject;

	result += ");\n		_ProcessEvent = decltype(_ProcessEvent)((uintptr_t)GetModuleHandle(0) + " + processevent + ");\n	};";

	return result;
}

void SDKFormatting::CreateSDKHeader(std::ofstream& header) {
	std::string StaticFindObjectOffset = "";
	std::string ProcessEventOffset = "";

	header << R"(
#pragma once

#include <Windows.h>
#include <string>
#include <locale>
#include <format>
#include <iostream>

namespace SDK {
	// https://github.com/EpicGames/UnrealEngine/blob/99b6e203a15d04fc7bbbf554c421a985c1ccb8f1/Engine/Source/Runtime/CoreUObject/Private/UObject/UObjectGlobals.cpp#L327
	static inline uintptr_t* (__fastcall* _StaticFindObject) (uintptr_t* ObjectClass, uintptr_t* InObjectPackage, const wchar_t* OrigInName, bool ExactClass) = 0;

	// https://github.com/EpicGames/UnrealEngine/blob/c3caf7b6bf12ae4c8e09b606f10a09776b4d1f38/Engine/Source/Runtime/CoreUObject/Private/UObject/ScriptCore.cpp#L1822
	static inline void* (__fastcall* _ProcessEvent) (uintptr_t* Object, uintptr_t* Function, void* Params) = 0;

)";

	
	header << GenerateInitFunction() << '\n';
	header << GenerateNameStruct() << '\n';
	header << GenerateTArray() << '\n';
	header << GenerateFString() << '\n';

	header << GenerateOthers() << '\n';

	header << "\n}\n\n\n";

	// Include CoreUObject
	header << std::format("#include \"{}\"\n", ("./Packages/" + std::string(SHORTNAME) + "_" + "CoreUObject_structs.hpp"));
	header << std::format("#include \"{}\"\n", ("./Packages/" + std::string(SHORTNAME) + "_" + "CoreUObject_classes.hpp"));
	header << std::format("#include \"{}\"\n", ("./Packages/" + std::string(SHORTNAME) + "_" + "CoreUObject_functions.cpp"));

}