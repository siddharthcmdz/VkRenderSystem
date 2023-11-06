/*
 *
 *  Copyright (C) 2008-2012, OFFIS e.V. and ICSMED AG, Oldenburg, Germany
 *  Copyright (C) 2013-2023, J. Riesmeier, Oldenburg, Germany
 *  All rights reserved.  See COPYRIGHT file for details.
 *
 *  Header file for class DRTReferencedBeamSequenceInRTFractionSchemeModule
 *
 *  Generated automatically from DICOM PS 3.3-2023b
 *  File created on 2023-05-19 16:00:57
 *
 */


#ifndef DRTRBS3_H
#define DRTRBS3_H

#include "dcmtk/config/osconfig.h"     // make sure OS specific configuration is included first

#include "dcmtk/ofstd/oflist.h"        // for standard list class
#include "dcmtk/dcmrt/drttypes.h"      // module-specific helper class
#include "dcmtk/dcmrt/seq/drtdccs.h"   // for DoseCalibrationConditionsSequence
#include "dcmtk/dcmrt/seq/drtrdcks.h"  // for RadiationDeviceConfigurationAndCommissioningKeySequence


/** Interface class for ReferencedBeamSequence (300c,0004) in RTFractionSchemeModule
 */
class DCMTK_DCMRT_EXPORT DRTReferencedBeamSequenceInRTFractionSchemeModule
  : protected DRTTypes
{

  public:

    /** Item class
     */
    class DCMTK_DCMRT_EXPORT Item
      : protected DRTTypes
    {

      public:

      // --- constructors, destructor and operators ---

        /** (default) constructor
         *  @param emptyDefaultItem flag used to mark the empty default item
         */
        Item(const OFBool emptyDefaultItem = OFFalse);

        /** copy constructor
         *  @param copy item object to be copied
         */
        Item(const Item &copy);

        /** destructor
         */
        virtual ~Item();

        /** assignment operator
         *  @param copy item object to be copied
         *  @return reference to this object
         */
        Item &operator=(const Item &copy);

      // --- general methods ---

        /** clear all internal member variables
         */
        void clear();

        /** check if item is empty
         *  @return OFTrue if item is empty, OFFalse otherwise
         */
        OFBool isEmpty();

        /** check if item is valid, i.e.\ not the empty default item
         *  @return OFTrue if item is valid, OFFalse otherwise
         */
        OFBool isValid() const;

      // --- input/output methods ---

        /** read elements from sequence item
         *  @param  item    reference to DICOM sequence item from which the elements should be read
         *  @return status, EC_Normal if successful, an error code otherwise
         */
        OFCondition read(DcmItem &item);

        /** write elements to sequence item
         *  @param  item    reference to DICOM sequence item to which the elements should be written
         *  @return status, EC_Normal if successful, an error code otherwise
         */
        OFCondition write(DcmItem &item);

      // --- get DICOM attribute values ---

        /** get AlternateBeamDose (300a,0091)
         *  @param  value  reference to variable in which the value should be stored
         *  @param  pos    index of the value to get (0..vm-1), -1 for all components
         *  @return status, EC_Normal if successful, an error code otherwise
         */
        OFCondition getAlternateBeamDose(OFString &value, const signed long pos = 0) const;

        /** get AlternateBeamDose (300a,0091)
         *  @param  value  reference to variable in which the value should be stored
         *  @param  pos    index of the value to get (0..vm-1)
         *  @return status, EC_Normal if successful, an error code otherwise
         */
        OFCondition getAlternateBeamDose(Float64 &value, const unsigned long pos = 0) const;

        /** get AlternateBeamDoseType (300a,0092)
         *  @param  value  reference to variable in which the value should be stored
         *  @param  pos    index of the value to get (0..vm-1), -1 for all components
         *  @return status, EC_Normal if successful, an error code otherwise
         */
        OFCondition getAlternateBeamDoseType(OFString &value, const signed long pos = 0) const;

        /** get BeamDeliveryDurationLimit (300a,00c5)
         *  @param  value  reference to variable in which the value should be stored
         *  @param  pos    index of the value to get (0..vm-1)
         *  @return status, EC_Normal if successful, an error code otherwise
         */
        OFCondition getBeamDeliveryDurationLimit(Float64 &value, const unsigned long pos = 0) const;

        /** get BeamDose (300a,0084)
         *  @param  value  reference to variable in which the value should be stored
         *  @param  pos    index of the value to get (0..vm-1), -1 for all components
         *  @return status, EC_Normal if successful, an error code otherwise
         */
        OFCondition getBeamDose(OFString &value, const signed long pos = 0) const;

        /** get BeamDose (300a,0084)
         *  @param  value  reference to variable in which the value should be stored
         *  @param  pos    index of the value to get (0..vm-1)
         *  @return status, EC_Normal if successful, an error code otherwise
         */
        OFCondition getBeamDose(Float64 &value, const unsigned long pos = 0) const;

        /** get BeamDoseType (300a,0090)
         *  @param  value  reference to variable in which the value should be stored
         *  @param  pos    index of the value to get (0..vm-1), -1 for all components
         *  @return status, EC_Normal if successful, an error code otherwise
         */
        OFCondition getBeamDoseType(OFString &value, const signed long pos = 0) const;

        /** get BeamMeterset (300a,0086)
         *  @param  value  reference to variable in which the value should be stored
         *  @param  pos    index of the value to get (0..vm-1), -1 for all components
         *  @return status, EC_Normal if successful, an error code otherwise
         */
        OFCondition getBeamMeterset(OFString &value, const signed long pos = 0) const;

        /** get BeamMeterset (300a,0086)
         *  @param  value  reference to variable in which the value should be stored
         *  @param  pos    index of the value to get (0..vm-1)
         *  @return status, EC_Normal if successful, an error code otherwise
         */
        OFCondition getBeamMeterset(Float64 &value, const unsigned long pos = 0) const;

        /** get DoseCalibrationConditionsVerifiedFlag (300c,0123)
         *  @param  value  reference to variable in which the value should be stored
         *  @param  pos    index of the value to get (0..vm-1), -1 for all components
         *  @return status, EC_Normal if successful, an error code otherwise
         */
        OFCondition getDoseCalibrationConditionsVerifiedFlag(OFString &value, const signed long pos = 0) const;

        /** get ReferencedBeamNumber (300c,0006)
         *  @param  value  reference to variable in which the value should be stored
         *  @param  pos    index of the value to get (0..vm-1), -1 for all components
         *  @return status, EC_Normal if successful, an error code otherwise
         */
        OFCondition getReferencedBeamNumber(OFString &value, const signed long pos = 0) const;

        /** get ReferencedBeamNumber (300c,0006)
         *  @param  value  reference to variable in which the value should be stored
         *  @param  pos    index of the value to get (0..vm-1)
         *  @return status, EC_Normal if successful, an error code otherwise
         */
        OFCondition getReferencedBeamNumber(Sint32 &value, const unsigned long pos = 0) const;

        /** get ReferencedDoseReferenceUID (300a,0083)
         *  @param  value  reference to variable in which the value should be stored
         *  @param  pos    index of the value to get (0..vm-1), -1 for all components
         *  @return status, EC_Normal if successful, an error code otherwise
         */
        OFCondition getReferencedDoseReferenceUID(OFString &value, const signed long pos = 0) const;

      // --- get DICOM sequence attributes ---

        /** get DoseCalibrationConditionsSequence (300c,0120)
         *  @return reference to sequence element
         */
        DRTDoseCalibrationConditionsSequence &getDoseCalibrationConditionsSequence()
            { return DoseCalibrationConditionsSequence; }

        /** get DoseCalibrationConditionsSequence (300c,0120)
         *  @return const reference to sequence element
         */
        const DRTDoseCalibrationConditionsSequence &getDoseCalibrationConditionsSequence() const
            { return DoseCalibrationConditionsSequence; }

        /** get RadiationDeviceConfigurationAndCommissioningKeySequence (300a,065a)
         *  @return reference to sequence element
         */
        DRTRadiationDeviceConfigurationAndCommissioningKeySequence &getRadiationDeviceConfigurationAndCommissioningKeySequence()
            { return RadiationDeviceConfigurationAndCommissioningKeySequence; }

        /** get RadiationDeviceConfigurationAndCommissioningKeySequence (300a,065a)
         *  @return const reference to sequence element
         */
        const DRTRadiationDeviceConfigurationAndCommissioningKeySequence &getRadiationDeviceConfigurationAndCommissioningKeySequence() const
            { return RadiationDeviceConfigurationAndCommissioningKeySequence; }

      // --- set DICOM attribute values ---

        /** set AlternateBeamDose (300a,0091)
         *  @param  value  value to be set (single value only) or "" for no value
         *  @param  check  check 'value' for conformance with VR (DS) and VM (1) if enabled
         *  @return status, EC_Normal if successful, an error code otherwise
         */
        OFCondition setAlternateBeamDose(const OFString &value, const OFBool check = OFTrue);

        /** set AlternateBeamDoseType (300a,0092)
         *  @param  value  value to be set (single value only) or "" for no value
         *  @param  check  check 'value' for conformance with VR (CS) and VM (1) if enabled
         *  @return status, EC_Normal if successful, an error code otherwise
         */
        OFCondition setAlternateBeamDoseType(const OFString &value, const OFBool check = OFTrue);

        /** set BeamDeliveryDurationLimit (300a,00c5)
         *  @param  value  value to be set (should be valid for this VR)
         *  @param  pos    index of the value to be set (0..vm-1), vm=1
         *  @return status, EC_Normal if successful, an error code otherwise
         */
        OFCondition setBeamDeliveryDurationLimit(const Float64 value, const unsigned long pos = 0);

        /** set BeamDose (300a,0084)
         *  @param  value  value to be set (single value only) or "" for no value
         *  @param  check  check 'value' for conformance with VR (DS) and VM (1) if enabled
         *  @return status, EC_Normal if successful, an error code otherwise
         */
        OFCondition setBeamDose(const OFString &value, const OFBool check = OFTrue);

        /** set BeamDoseType (300a,0090)
         *  @param  value  value to be set (single value only) or "" for no value
         *  @param  check  check 'value' for conformance with VR (CS) and VM (1) if enabled
         *  @return status, EC_Normal if successful, an error code otherwise
         */
        OFCondition setBeamDoseType(const OFString &value, const OFBool check = OFTrue);

        /** set BeamMeterset (300a,0086)
         *  @param  value  value to be set (single value only) or "" for no value
         *  @param  check  check 'value' for conformance with VR (DS) and VM (1) if enabled
         *  @return status, EC_Normal if successful, an error code otherwise
         */
        OFCondition setBeamMeterset(const OFString &value, const OFBool check = OFTrue);

        /** set DoseCalibrationConditionsVerifiedFlag (300c,0123)
         *  @param  value  value to be set (single value only) or "" for no value
         *  @param  check  check 'value' for conformance with VR (CS) and VM (1) if enabled
         *  @return status, EC_Normal if successful, an error code otherwise
         */
        OFCondition setDoseCalibrationConditionsVerifiedFlag(const OFString &value, const OFBool check = OFTrue);

        /** set ReferencedBeamNumber (300c,0006)
         *  @param  value  value to be set (single value only) or "" for no value
         *  @param  check  check 'value' for conformance with VR (IS) and VM (1) if enabled
         *  @return status, EC_Normal if successful, an error code otherwise
         */
        OFCondition setReferencedBeamNumber(const OFString &value, const OFBool check = OFTrue);

        /** set ReferencedDoseReferenceUID (300a,0083)
         *  @param  value  value to be set (single value only) or "" for no value
         *  @param  check  check 'value' for conformance with VR (UI) and VM (1) if enabled
         *  @return status, EC_Normal if successful, an error code otherwise
         */
        OFCondition setReferencedDoseReferenceUID(const OFString &value, const OFBool check = OFTrue);

      private:

        /// internal flag used to mark the empty default item
        /*const*/ OFBool EmptyDefaultItem;

        /// AlternateBeamDose (300a,0091) vr=DS, vm=1, type=3
        DcmDecimalString AlternateBeamDose;
        /// AlternateBeamDoseType (300a,0092) vr=CS, vm=1, type=1C
        DcmCodeString AlternateBeamDoseType;
        /// BeamDeliveryDurationLimit (300a,00c5) vr=FD, vm=1, type=3
        DcmFloatingPointDouble BeamDeliveryDurationLimit;
        /// BeamDose (300a,0084) vr=DS, vm=1, type=3
        DcmDecimalString BeamDose;
        /// BeamDoseType (300a,0090) vr=CS, vm=1, type=1C
        DcmCodeString BeamDoseType;
        /// BeamMeterset (300a,0086) vr=DS, vm=1, type=3
        DcmDecimalString BeamMeterset;
        /// DoseCalibrationConditionsSequence (300c,0120) vr=SQ, vm=1, type=1C
        DRTDoseCalibrationConditionsSequence DoseCalibrationConditionsSequence;
        /// DoseCalibrationConditionsVerifiedFlag (300c,0123) vr=CS, vm=1, type=3
        DcmCodeString DoseCalibrationConditionsVerifiedFlag;
        /// RadiationDeviceConfigurationAndCommissioningKeySequence (300a,065a) vr=SQ, vm=1, type=1C
        DRTRadiationDeviceConfigurationAndCommissioningKeySequence RadiationDeviceConfigurationAndCommissioningKeySequence;
        /// ReferencedBeamNumber (300c,0006) vr=IS, vm=1, type=1
        DcmIntegerString ReferencedBeamNumber;
        /// ReferencedDoseReferenceUID (300a,0083) vr=UI, vm=1, type=3
        DcmUniqueIdentifier ReferencedDoseReferenceUID;

    };

  // --- constructors, destructor and operators ---

    /** (default) constructor
     *  @param emptyDefaultSequence internal flag used to mark the empty default sequence
     */
    DRTReferencedBeamSequenceInRTFractionSchemeModule(const OFBool emptyDefaultSequence = OFFalse);

    /** copy constructor
     *  @param copy sequence object to be copied
     */
    DRTReferencedBeamSequenceInRTFractionSchemeModule(const DRTReferencedBeamSequenceInRTFractionSchemeModule &copy);

    /** destructor
     */
    virtual ~DRTReferencedBeamSequenceInRTFractionSchemeModule();

    /** assignment operator
     *  @param copy sequence object to be copied
     *  @return reference to this object
     */
    DRTReferencedBeamSequenceInRTFractionSchemeModule &operator=(const DRTReferencedBeamSequenceInRTFractionSchemeModule &copy);

  // --- general methods ---

    /** clear all internal member variables
     */
    void clear();

    /** check if sequence is empty
     *  @return OFTrue if sequence is empty, OFFalse otherwise
     */
    OFBool isEmpty();

    /** check if sequence is valid, i.e.\ not the empty default sequence
     *  @return OFTrue if sequence is valid, OFFalse otherwise
     */
    OFBool isValid() const;

    /** get number of items in the sequence
     *  @return number of items
     */
    size_t getNumberOfItems() const;

    /** goto first item in the sequence
     *  @return status, EC_Normal if successful, an error code otherwise
     */
    OFCondition gotoFirstItem();

    /** goto next item in the sequence
     *  @return status, EC_Normal if successful, an error code otherwise
     */
    OFCondition gotoNextItem();

    /** goto particular item in the sequence
     *  @param  num  number of the item to be selected (0..num-1)
     *  @return status, EC_Normal if successful, an error code otherwise
     */
    OFCondition gotoItem(const size_t num);

    /** get current item in the sequence
     *  @param  item  reference to item pointer (result variable)
     *  @return status, EC_Normal if successful, an error code otherwise
     */
    OFCondition getCurrentItem(Item *&item) const;

    /** get current item in the sequence
     *  @return reference to specified item if successful, empty default item otherwise
     */
    Item &getCurrentItem();

    /** get current item in the sequence
     *  @return const reference to specified item if successful, empty default item otherwise
     */
    const Item &getCurrentItem() const;

    /** get particular item in the sequence
     *  @param  num   number of the item to be retrieved (0..num-1)
     *  @param  item  reference to item pointer (result variable)
     *  @return status, EC_Normal if successful, an error code otherwise
     */
    OFCondition getItem(const size_t num, Item *&item);

    /** get particular item in the sequence
     *  @param  num  number of the item to be retrieved (0..num-1)
     *  @return reference to specified item if successful, empty default item otherwise
     */
    Item &getItem(const size_t num);

    /** get particular item in the sequence
     *  @param  num  number of the item to be retrieved (0..num-1)
     *  @return const reference to specified item if successful, empty default item otherwise
     */
    const Item &getItem(const size_t num) const;

    /** get particular item in the sequence
     *  @param  num  number of the item to be retrieved (0..num-1)
     *  @return reference to specified item if successful, empty default item otherwise
     */
    Item &operator[](const size_t num);

    /** get particular item in the sequence
     *  @param  num  number of the item to be retrieved (0..num-1)
     *  @return const reference to specified item if successful, empty default item otherwise
     */
    const Item &operator[](const size_t num) const;

    /** create and add new item to the end of this sequence
     *  @param  item  reference to new item pointer (result variable)
     *  @return status, EC_Normal if successful, an error code otherwise
     */
    OFCondition addItem(Item *&item);

    /** create and insert new item into the sequence
     *  @param  pos   position where the new item is to be inserted (0..num)
     *  @param  item  reference to new item pointer (result variable)
     *  @return status, EC_Normal if successful, an error code otherwise
     */
    OFCondition insertItem(const size_t pos, Item *&item);

    /** remove particular item from the sequence
     *  @param  pos  position of the item to be removed (0..num-1)
     *  @return status, EC_Normal if successful, an error code otherwise
     */
    OFCondition removeItem(const size_t pos);

  // --- input/output methods ---

    /** read sequence of items from dataset
     *  @param  dataset     reference to DICOM dataset from which the sequence should be read
     *  @param  card        cardinality (valid range for number of items)
     *  @param  type        value type (valid value: "1", "1C", "2" or something else which is not checked)
     *  @param  moduleName  optional module/sequence name to be printed (default: "RT object" if NULL)
     *  @return status, EC_Normal if successful, an error code otherwise
     */
    OFCondition read(DcmItem &dataset,
                     const OFString &card,
                     const OFString &type,
                     const char *moduleName = NULL);

    /** write sequence of items to dataset
     *  @param  dataset     reference to DICOM dataset to which the sequence should be written
     *  @param  card        cardinality (valid range for number of items)
     *  @param  type        value type (valid value: "1", "2" or something else which is not checked)
     *  @param  moduleName  optional module/sequence name to be printed (default: "RT object" if NULL)
     *  @return status, EC_Normal if successful, an error code otherwise
     */
    OFCondition write(DcmItem &dataset,
                      const OFString &card,
                      const OFString &type,
                      const char *moduleName = NULL);

  protected:

    /** goto particular item in the sequence
     *  @param  num       number of the item to be selected (0..num-1)
     *  @param  iterator  list iterator storing the position of the item
     *  @return status, EC_Normal if successful, an error code otherwise
     */
    OFCondition gotoItem(const size_t num,
                         OFListIterator(Item *) &iterator);

    /** goto particular item in the sequence
     *  @param  num       number of the item to be selected (0..num-1)
     *  @param  iterator  list iterator storing the position of the item
     *  @return status, EC_Normal if successful, an error code otherwise
     */
    OFCondition gotoItem(const size_t num,
                         OFListConstIterator(Item *) &iterator) const;

  private:

    /// internal flag used to mark the empty default sequence
    /*const*/ OFBool EmptyDefaultSequence;

    /// list of items in this sequence
    OFList<Item *>         SequenceOfItems;
    /// currently selected item
    OFListIterator(Item *) CurrentItem;
    /// empty default item
    Item                   EmptyItem;

};


#endif
