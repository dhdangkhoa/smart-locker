enum CompartmentStatus {available, occupied, maintenance}

class CompartmentModel {
  final String id;
  final String boxNumber;
  final CompartmentStatus status;
  final String? trackingCode;

  CompartmentModel({
    required this.id,
    required this.boxNumber,
    required this.status,
    this.trackingCode,
  });

}