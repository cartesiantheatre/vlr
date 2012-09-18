/*
    VikingExtractor, to recover images from Viking Lander operations.
    Copyright (C) 2010, 2011, 2012 Cartesian Theatre <kip@thevertigo.com>.
    
    Public discussion on IRC available at #avaneya (irc.freenode.net) 
    or on the mailing list <avaneya@lists.avaneya.com>.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// Multiple include protection...
#ifndef _EXPLICIT_SINGLETON_H_
#define _EXPLICIT_SINGLETON_H_

// Includes...

    // System headers...
    #include <cassert>

// Explicit singleton pattern whose creation and deconstruction are managed by
//  the client. Subclasses must declare themselves a friend of this pattern in 
//  order to protect themselves from being manually instantiated on the stack...
template <class SingletonType>
class ExplicitSingleton
{
    // Public methods...
    public:

        // Create the object...
        static SingletonType &CreateSingleton()
        {
            // Runtime error if already created...
            assert(!IsInstantiated());

            // Construct and return the new instance...
            ms_Instance = new SingletonType;
            return *ms_Instance;
        }

        // Destroy the object...
        static void DestroySingleton()
        {
            // Runtime error if it did not already exist...
            assert(IsInstantiated());
            
            // Deconstruct...
            delete ms_Instance;
            ms_Instance = 0;
        }
        
        // Get the instance. Error if called and it didn't already exist...
        static SingletonType &GetInstance()
        {
            assert(IsInstantiated());
            return *ms_Instance;
        }

        // Get the instance. Error if called and it didn't already exist...
        static SingletonType *GetInstancePointer()
        {
            assert(IsInstantiated());
            return ms_Instance;
        }

        // Check if the singleton is instantiated...
        static bool IsInstantiated() { return ms_Instance ? true : false; }

    // Protected methods...
    protected:

        // Protected constructor forbids clients from manually instantiating...
        ExplicitSingleton()
        {
            assert(!IsInstantiated());
        }

        // Deconstructor...
       ~ExplicitSingleton()
        {
        }

    // Private methods...
    private:

        // Forbid copy constructor and assignment...
        ExplicitSingleton(const ExplicitSingleton<SingletonType> &);
        ExplicitSingleton &operator=(const ExplicitSingleton &);
        
        // The actual object instance...
        static SingletonType *ms_Instance;
};

// Start by marking the singleton as uninitialized...
template <class SingletonType>
SingletonType *ExplicitSingleton<SingletonType>::ms_Instance = 0;

/*
template <class SubsystemType>
class SubsystemManagerBase : public ExplicitSingleton<SubsystemType>
{
    public:

        const std::wstring &GetFriendlyName() const
        {
            return m_FriendlyName;
        }

        static SubsystemType &Initialize()
        {
            SubsystemType &Subsystem = SubsystemManagerBase<SubsystemType>::CreateSingleton();
            
//            wcout << L"Initialized subsystem " << Subsystem.GetFriendlyName() << endl;
            return Subsystem;
        }

        static bool IsInitialized()
        {
            return SubsystemManagerBase<SubsystemType>::IsInstantiated();
        }
        
        static void Shutdown()
        {
//            SubsystemType &Subsystem = SubsystemManagerBase<SubsystemType>::GetSingleton();
//            wcout << L"Shutting down subsystem " << Subsystem.GetFriendlyName() << endl;
            SubsystemManagerBase<SubsystemType>::DestroySingleton();
        }
        
    protected:

        SubsystemManagerBase(const wstring &FriendlyName)
            : m_FriendlyName(FriendlyName)
        {
//            cout << __PRETTY_FUNCTION__ << endl;
        }

        virtual ~SubsystemManagerBase()
        {

        }

        wstring m_FriendlyName;
};

class Foo : public SubsystemManagerBase<Foo>
{
    public:

        Foo()
            : SubsystemManagerBase<Foo>(L"FooSubsystem")
        {
//            cout << __PRETTY_FUNCTION__ << " (" << this << ")" << endl;
            wcout << GetFriendlyName() << L" has been initialized." << endl;
        }
        
        void Print() const
        {
//            cout << __PRETTY_FUNCTION__ << " (" << this << ")" << endl;
        }

       ~Foo()
        {
//            cout << __PRETTY_FUNCTION__ << " (" << this << ")" << endl;
            wcout << GetFriendlyName() << L" has been shutdown." << endl;
        }
};

class Poo : public SubsystemManagerBase<Poo>
{
    public:

        Poo()
            : SubsystemManagerBase<Poo>(L"PooSubsystem")
        {
//            cout << __PRETTY_FUNCTION__ << " (" << this << ")" << endl;
            wcout << GetFriendlyName() << L" has been initialized." << endl;
        }
        
        void Print() const
        {
//            cout << __PRETTY_FUNCTION__ << " (" << this << ")" << endl;
        }

       ~Poo()
        {
//            cout << __PRETTY_FUNCTION__ << " (" << this << ")" << endl;
            wcout << GetFriendlyName() << L" has been shutdown." << endl;
        }
};

*/

#endif

